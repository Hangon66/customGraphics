#include "PropertyPanel.h"
#include "ShapeMetadata.h"
#include "../handlers/DrawHandler.h"

#include <QLabel>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsRectItem>
#include <QGraphicsLineItem>
#include <QGraphicsItem>
#include <QScrollArea>
#include <QSet>

PropertyPanel::PropertyPanel(QWidget *parent)
    : QWidget(parent)
    , m_updating(false)
    , m_currentItem(nullptr)
    , m_titleLabel(nullptr)
    , m_nameEdit(nullptr)
    , m_toggleButton(nullptr)
    , m_toggleBar(nullptr)
    , m_panelBody(nullptr)
    , m_contentWidget(nullptr)
    , m_propsLayout(nullptr)
    , m_noSelectionLabel(nullptr)
{
    initUI();
}

PropertyPanel::~PropertyPanel()
{
}

void PropertyPanel::initUI()
{
    // 顶层水平布局：左侧折叠按钮条 + 右侧面板主体
    QHBoxLayout *topLayout = new QHBoxLayout(this);
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->setSpacing(0);

    // ====== 左侧折叠/展开按钮条（始终可见）======
    m_toggleBar = new QWidget(this);
    m_toggleBar->setFixedWidth(20);

    QVBoxLayout *toggleBarLayout = new QVBoxLayout(m_toggleBar);
    toggleBarLayout->setContentsMargins(0, 0, 0, 0);
    toggleBarLayout->setSpacing(0);

    m_toggleButton = new QPushButton(m_toggleBar);
    m_toggleButton->setFixedWidth(18);
    m_toggleButton->setFixedHeight(24);
    m_toggleButton->setText("<");

    connect(m_toggleButton, &QPushButton::clicked, this, &PropertyPanel::togglePanel);
    toggleBarLayout->addWidget(m_toggleButton);
    toggleBarLayout->addStretch();

    topLayout->addWidget(m_toggleBar);

    // ====== 右侧面板主体（折叠时隐藏）======
    m_panelBody = new QWidget(this);
    m_panelBody->setFixedWidth(200);
    QVBoxLayout *panelBodyLayout = new QVBoxLayout(m_panelBody);
    panelBodyLayout->setContentsMargins(0, 0, 0, 0);
    panelBodyLayout->setSpacing(0);

    // 标题栏
    QWidget *titleBar = new QWidget(m_panelBody);
    titleBar->setFixedHeight(30);

    QHBoxLayout *titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(8, 0, 4, 0);

    m_titleLabel = new QLabel("属性", titleBar);

    titleLayout->addWidget(m_titleLabel);
    titleLayout->addStretch();

    panelBodyLayout->addWidget(titleBar);

    // 内容区域（含名称 + 动态属性行）
    QScrollArea *scrollArea = new QScrollArea(m_panelBody);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    m_contentWidget = new QWidget(scrollArea);
    m_contentWidget->setStyleSheet(
        "QWidget { background-color: #1e1e1e; }"
        "QLineEdit { background-color: #000000; }"
        "QDoubleSpinBox { background-color: #000000; }");
    QVBoxLayout *contentLayout = new QVBoxLayout(m_contentWidget);
    contentLayout->setContentsMargins(8, 8, 8, 8);
    contentLayout->setSpacing(6);

    // 名称（始终显示）
    QHBoxLayout *nameLayout = new QHBoxLayout();
    QLabel *nameLabel = new QLabel("名称:", m_contentWidget);
    nameLabel->setFixedWidth(50);
    m_nameEdit = new QLineEdit(m_contentWidget);
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(m_nameEdit);
    contentLayout->addLayout(nameLayout);

    // 动态属性行容器
    m_propsLayout = new QVBoxLayout();
    m_propsLayout->setSpacing(4);
    contentLayout->addLayout(m_propsLayout);

    contentLayout->addStretch();

    scrollArea->setWidget(m_contentWidget);
    panelBodyLayout->addWidget(scrollArea);

    // 未选中提示
    m_noSelectionLabel = new QLabel("未选中图元", m_panelBody);
    m_noSelectionLabel->setAlignment(Qt::AlignCenter);

    panelBodyLayout->addWidget(m_noSelectionLabel);

    topLayout->addWidget(m_panelBody);

    // 连接名称编辑信号
    connect(m_nameEdit, &QLineEdit::editingFinished, this, &PropertyPanel::onNameEditingFinished);

    // 初始状态：展开面板
    setPanelVisible(true);
}

void PropertyPanel::updateFromItem(QGraphicsItem *item)
{
    m_updating = true;
    m_currentItem = item;

    if (!item) {
        clearPanel();
        m_updating = false;
        return;
    }

    // 显示属性内容，隐藏未选中提示
    m_contentWidget->setVisible(m_panelBody->isVisible());
    m_noSelectionLabel->setVisible(false);

    // 从 data() 读取元数据
    PropMap props = item->data(ShapeMeta::Props).value<PropMap>();

    // 从图元实际几何刷新 PropMap 中的几何属性值
    refreshGeometryProps(item, props);

    // 将刷新后的 PropMap 写回
    item->setData(ShapeMeta::Props, QVariant::fromValue(props));

    // 名称：直接从 data(Name) 读取，创建时已写入
    QString name = item->data(ShapeMeta::Name).toString();
    m_nameEdit->setText(name);

    // 判断是否需要重建控件：键列表变化时重建，否则仅更新值
    QStringList newKeys = props.keys();
    if (newKeys == m_lastPropKeys && !m_propWidgets.isEmpty()) {
        // 键未变，仅刷新控件显示值（拖动等高频场景）
        updatePropertyValues(props);
    } else {
        // 键变化（切换图元/首次选中），重建控件
        buildPropertyRows(props);
        m_lastPropKeys = newKeys;
    }

    m_updating = false;
}

void PropertyPanel::buildPropertyRows(const QMap<QString, PropField> &props)
{
    clearPropertyRows();
    m_propWidgets.clear();

    // 自定义属性行显示顺序：名称由专用控件显示，这里按 类型→坐标→尺寸→其他 排列
    static const QStringList preferredOrder = {
        "typeName", "x", "y", "length", "height", "width"
    };

    // 收集已排序键和剩余键
    QStringList sortedKeys;
    QSet<QString> visited;
    for (const QString &key : preferredOrder) {
        if (props.contains(key) && props[key].isVisible()) {
            sortedKeys.append(key);
            visited.insert(key);
        }
    }
    for (auto it = props.constBegin(); it != props.constEnd(); ++it) {
        if (!visited.contains(it.key()) && it.value().isVisible()) {
            sortedKeys.append(it.key());
        }
    }

    for (const QString &key : sortedKeys) {
        const PropField &field = props[key];

        QHBoxLayout *rowLayout = new QHBoxLayout();
        rowLayout->setSpacing(4);

        QLabel *label = new QLabel(field.displayName(), m_contentWidget);
        label->setFixedWidth(50);
        rowLayout->addWidget(label);

        // 根据 PropType 选择控件
        if (field.propType() == PropType::Number) {
            QDoubleSpinBox *spin = new QDoubleSpinBox(m_contentWidget);
            spin->setRange(-99999, 99999);
            spin->setDecimals(1);
            spin->setSingleStep(1.0);
            spin->setValue(field.toDouble());
            spin->setEnabled(field.isEditable());
            connect(spin, &QDoubleSpinBox::editingFinished, this, [this, key, spin]() {
                if (m_updating || !m_currentItem) return;
                qDebug() << "[PropertyPanel] spin editingFinished: key=" << key << "value=" << spin->value();
                PropMap props = m_currentItem->data(ShapeMeta::Props).value<PropMap>();
                if (props.contains(key)) {
                    props[key].setValue(spin->value());
                    m_currentItem->setData(ShapeMeta::Props, QVariant::fromValue(props));
                }
                emit propertyChanged(m_currentItem, key, spin->value());
            });
            rowLayout->addWidget(spin);
            m_dynamicWidgets.append(spin);
            m_propWidgets[key] = spin;
        } else if (field.propType() == PropType::Bool) {
            QLineEdit *edit = new QLineEdit(field.toBool() ? "true" : "false", m_contentWidget);
            edit->setEnabled(false);
            rowLayout->addWidget(edit);
            m_dynamicWidgets.append(edit);
            m_propWidgets[key] = edit;
        } else {
            // PropType::Text 使用 QLineEdit
            QLineEdit *edit = new QLineEdit(field.toString(), m_contentWidget);
            edit->setEnabled(field.isEditable());
            connect(edit, &QLineEdit::editingFinished, this, [this, key, edit]() {
                if (m_updating || !m_currentItem) return;
                qDebug() << "[PropertyPanel] text editingFinished: key=" << key << "value=" << edit->text();
                PropMap props = m_currentItem->data(ShapeMeta::Props).value<PropMap>();
                if (props.contains(key)) {
                    props[key].setValue(edit->text());
                    m_currentItem->setData(ShapeMeta::Props, QVariant::fromValue(props));
                }
                emit propertyChanged(m_currentItem, key, edit->text());
            });
            rowLayout->addWidget(edit);
            m_dynamicWidgets.append(edit);
            m_propWidgets[key] = edit;
        }

        m_dynamicWidgets.append(label);

        // 创建容器 widget 以便管理布局
        QWidget *rowWidget = new QWidget(m_contentWidget);
        rowWidget->setLayout(rowLayout);
        m_propsLayout->addWidget(rowWidget);
        m_dynamicWidgets.append(rowWidget);
    }
}

void PropertyPanel::clearPropertyRows()
{
    // 移除动态控件
    for (QWidget *w : m_dynamicWidgets) {
        m_propsLayout->removeWidget(w);
        delete w;
    }
    m_dynamicWidgets.clear();
    m_propWidgets.clear();
    m_lastPropKeys.clear();
}

void PropertyPanel::clearPanel()
{
    m_currentItem = nullptr;
    m_contentWidget->setVisible(false);
    m_noSelectionLabel->setVisible(true);
    m_nameEdit->clear();
    clearPropertyRows();
}

void PropertyPanel::setPanelVisible(bool visible)
{
    m_panelBody->setVisible(visible);
    m_contentWidget->setVisible(visible && m_currentItem != nullptr);
    m_noSelectionLabel->setVisible(visible && m_currentItem == nullptr);
    m_toggleButton->setText(visible ? ">" : "<");
}

bool PropertyPanel::isPanelVisible() const
{
    return m_panelBody->isVisible();
}

void PropertyPanel::togglePanel()
{
    setPanelVisible(!isPanelVisible());
}

void PropertyPanel::onNameEditingFinished()
{
    if (m_updating || !m_currentItem) {
        return;
    }

    QString newName = m_nameEdit->text();

    // 写回标签文本（LabeledRectItem/LabeledLineItem）
    if (auto *rectItem = dynamic_cast<LabeledRectItem*>(m_currentItem)) {
        rectItem->setLabelText(newName);
    } else if (auto *lineItem = dynamic_cast<LabeledLineItem*>(m_currentItem)) {
        lineItem->setLabelText(newName);
    }
    // 统一写入 data(Name)
    m_currentItem->setData(ShapeMeta::Name, newName);

    emit propertyChanged(m_currentItem, "name", newName);
}

void PropertyPanel::refreshGeometryProps(QGraphicsItem *item, PropMap &props)
{
    if (!item) {
        return;
    }

    // 矩形类型：视觉位置 = pos() + rect().topLeft()
    if (auto *rectItem = dynamic_cast<QGraphicsRectItem*>(item)) {
        QRectF r = rectItem->rect();
        QPointF visualPos = rectItem->pos() + r.topLeft();
        if (props.contains("x")) {
            props["x"].setValue(visualPos.x());
        } else {
            props["x"] = PropField(visualPos.x(), "X:", PropType::Number, true, true);
        }
        if (props.contains("y")) {
            props["y"].setValue(visualPos.y());
        } else {
            props["y"] = PropField(visualPos.y(), "Y:", PropType::Number, true, true);
        }
        if (props.contains("width")) {
            props["width"].setValue(r.width());
        } else {
            props["width"] = PropField(r.width(), "宽度:", PropType::Number, true, true);
        }
        if (props.contains("height")) {
            props["height"].setValue(r.height());
        } else {
            props["height"] = PropField(r.height(), "高度:", PropType::Number, true, true);
        }
    }
    // 线条类型：视觉位置 = pos() + line().p1()
    else if (auto *lineItem = dynamic_cast<QGraphicsLineItem*>(item)) {
        QLineF l = lineItem->line();
        QPointF visualPos = lineItem->pos() + l.p1();
        if (props.contains("x")) {
            props["x"].setValue(visualPos.x());
        } else {
            props["x"] = PropField(visualPos.x(), "X:", PropType::Number, true, true);
        }
        if (props.contains("y")) {
            props["y"].setValue(visualPos.y());
        } else {
            props["y"] = PropField(visualPos.y(), "Y:", PropType::Number, true, true);
        }
        if (props.contains("length")) {
            props["length"].setValue(l.length());
        } else {
            props["length"] = PropField(l.length(), "长度:", PropType::Number, true, true);
        }
    }
}

void PropertyPanel::updatePropertyValues(const PropMap &props)
{
    // 使用与 buildPropertyRows 相同的排序逻辑
    static const QStringList preferredOrder = {
        "typeName", "x", "y", "length", "height", "width"
    };

    QStringList sortedKeys;
    QSet<QString> visited;
    for (const QString &key : preferredOrder) {
        if (props.contains(key) && props[key].isVisible()) {
            sortedKeys.append(key);
            visited.insert(key);
        }
    }
    for (auto it = props.constBegin(); it != props.constEnd(); ++it) {
        if (!visited.contains(it.key()) && it.value().isVisible()) {
            sortedKeys.append(it.key());
        }
    }

    for (const QString &key : sortedKeys) {
        const PropField &field = props[key];

        QWidget *widget = m_propWidgets.value(key, nullptr);
        if (!widget) {
            continue;
        }

        if (field.propType() == PropType::Number) {
            QDoubleSpinBox *spin = qobject_cast<QDoubleSpinBox*>(widget);
            if (spin && !qFuzzyCompare(spin->value(), field.toDouble())) {
                spin->setValue(field.toDouble());
            }
        } else if (field.propType() == PropType::Text) {
            QLineEdit *edit = qobject_cast<QLineEdit*>(widget);
            if (edit && edit->text() != field.toString()) {
                edit->setText(field.toString());
            }
        }
        // Bool 类型只读，无需更新
    }
}
