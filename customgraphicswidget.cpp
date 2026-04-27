#include "customgraphicswidget.h"
#include "ui_customgraphicswidget.h"
#include "view/CustomGraphicsView.h"
#include "view/CustomGraphicsScene.h"
#include "handlers/SceneConfig.h"
#include "handlers/BackgroundHandler.h"
#include "handlers/RulerHandler.h"
#include "handlers/GuideLineHandler.h"
#include "handlers/DrawHandler.h"
#include "handlers/CollisionHandler.h"
#include "commands/ShapeCommands.h"
#include "view/PropertyPanel.h"
#include "view/MinimapWidget.h"
#include "view/ShapeMetadata.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsRectItem>
#include <QGraphicsLineItem>
#include <QPen>
#include <QBrush>
#include <QDebug>
#include <QPushButton>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QKeyEvent>
#include <QUndoStack>

CustomGraphicsWidget::CustomGraphicsWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CustomGraphicsWidget)
    , m_scene(nullptr)
    , m_view(nullptr)
    , m_backgroundHandler(nullptr)
    , m_rulerHandler(nullptr)
    , m_guideLineHandler(nullptr)
    , m_drawHandler(nullptr)
    , m_modeButton(nullptr)
    , m_modeLabel(nullptr)
    , m_undoButton(nullptr)
    , m_redoButton(nullptr)
    , m_toolBar(nullptr)
    , m_propertyPanel(nullptr)
    , m_selectedGuideLineIndex(-1)
{
    ui->setupUi(this);
    m_propertyPanel = new PropertyPanel(this);
    m_minimapWidget = new MinimapWidget(this);
    initStoneCuttingScene();
    initToolBar();
}

CustomGraphicsWidget::~CustomGraphicsWidget()
{
    delete ui;
}

void CustomGraphicsWidget::keyPressEvent(QKeyEvent *event)
{
    QWidget::keyPressEvent(event);
}

void CustomGraphicsWidget::toggleDrawSelectMode()
{
    if (m_drawHandler) {
        m_drawHandler->toggleDrawingMode();
        updateModeDisplay();
    }
}

void CustomGraphicsWidget::setDrawMode()
{
    if (m_drawHandler && !m_drawHandler->isDrawingActive()) {
        m_drawHandler->setDrawingActive(true);
        updateModeDisplay();
    }
}

void CustomGraphicsWidget::setSelectMode()
{
    if (m_drawHandler && m_drawHandler->isDrawingActive()) {
        m_drawHandler->setDrawingActive(false);
        updateModeDisplay();
    }
}

void CustomGraphicsWidget::undo()
{
    if (m_view) {
        m_view->undo();
    }
}

void CustomGraphicsWidget::redo()
{
    if (m_view) {
        m_view->redo();
    }
}

bool CustomGraphicsWidget::isDrawingActive() const
{
    return m_drawHandler ? m_drawHandler->isDrawingActive() : false;
}

void CustomGraphicsWidget::setToolBarVisible(bool visible)
{
    if (m_toolBar) {
        m_toolBar->setVisible(visible);
    }
}

bool CustomGraphicsWidget::isToolBarVisible() const
{
    return m_toolBar ? m_toolBar->isVisible() : false;
}

void CustomGraphicsWidget::initStoneCuttingScene()
{
    // 获取石材切割场景配置
    SceneConfig config = SceneConfigFactory::createStoneCuttingConfig();

    // 创建场景
    m_scene = new CustomGraphicsScene(this);

    // 配置碰撞检测：仅启用矩形间碰撞，边距和缩放因子从场景配置读取
    CollisionConfig collisionConfig;
    collisionConfig.enableRectOnlyCollision();
    collisionConfig.setMmToSceneScale(config.mmToSceneScale);
    collisionConfig.setCollisionMarginMM(config.collisionMarginMM);
    m_scene->setCollisionConfig(collisionConfig);
    m_scene->setCollisionEnabled(true);

    // 创建视图
    m_view = new CustomGraphicsView(this);
    m_view->setScene(m_scene);

    // 根据配置创建并添加 Handler
    QList<IInteractionHandler*> handlers = config.createHandlers();
    for (IInteractionHandler* handler : handlers) {
        m_view->addHandler(handler);

        // 保存特定 Handler 的引用以便后续使用
        if (auto* bgHandler = dynamic_cast<BackgroundHandler*>(handler)) {
            m_backgroundHandler = bgHandler;
            m_backgroundHandler->setScene(m_scene);
        }
        else if (auto* rulerHandler = dynamic_cast<RulerHandler*>(handler)) {
            m_rulerHandler = rulerHandler;
        }
        else if (auto* drawHandler = dynamic_cast<DrawHandler*>(handler)) {
            m_drawHandler = drawHandler;
            m_drawHandler->setScene(m_scene);
            // 设置撤销栈，支持撤销/重做
            m_drawHandler->setUndoStack(m_view->undoStack());
        }
    }

    // 创建并注册辅助线处理器
    m_guideLineHandler = new GuideLineHandler(80, this);
    if (m_rulerHandler) {
        m_guideLineHandler->setRulerWidth(m_rulerHandler->rulerWidth());
        // 根据标尺位置设置辅助线拖拽区域
        if (m_rulerHandler->rulerPosition() == RulerHandler::RulerPosition::Top) {
            m_guideLineHandler->setRulerPosition(GuideLineHandler::RulerPosition::Top);
        } else {
            m_guideLineHandler->setRulerPosition(GuideLineHandler::RulerPosition::Bottom);
        }
    }
    m_view->addHandler(m_guideLineHandler);

    // 连接撤销栈变化信号，更新按钮状态
    connect(m_view->undoStack(), &QUndoStack::canUndoChanged,
            this, &CustomGraphicsWidget::updateUndoRedoState);
    connect(m_view->undoStack(), &QUndoStack::canRedoChanged,
            this, &CustomGraphicsWidget::updateUndoRedoState);

    // 连接信号槽
    connectHandlers();

    // 添加测试图元
    addTestItems();

    qDebug() << "石材切割场景初始化完成:"
             << "标尺精度=" << config.rulerUnitPixel << "px/"
             << config.rulerUnitName;
}

void CustomGraphicsWidget::initToolBar()
{
    // 创建工具栏容器
    m_toolBar = new QWidget(this);
    m_toolBar->setFixedHeight(40);
    m_toolBar->setStyleSheet("background-color: #f0f0f0; border-bottom: 1px solid #ccc;");

    QHBoxLayout *toolLayout = new QHBoxLayout(m_toolBar);
    toolLayout->setContentsMargins(10, 5, 10, 5);
    toolLayout->setSpacing(10);

    // 模式切换按钮
    m_modeButton = new QPushButton("绘制模式", m_toolBar);
    m_modeButton->setCheckable(true);
    m_modeButton->setChecked(true);
    m_modeButton->setFixedWidth(100);
    m_modeButton->setStyleSheet(
        "QPushButton { padding: 5px 10px; border-radius: 3px; border: none; }"
        "QPushButton:checked { background-color: #4CAF50; color: white; }"
        "QPushButton:checked:hover { background-color: #43A047; }"
        "QPushButton:checked:pressed { background-color: #388E3C; }"
        "QPushButton:!checked { background-color: #2196F3; color: white; }"
        "QPushButton:!checked:hover { background-color: #1E88E5; }"
        "QPushButton:!checked:pressed { background-color: #1976D2; }");
    connect(m_modeButton, &QPushButton::clicked, this, &CustomGraphicsWidget::toggleDrawSelectMode);

    // 撤销按钮 - 可用时橙色，表示"回退"操作
    m_undoButton = new QPushButton("撤销", m_toolBar);
    m_undoButton->setFixedWidth(60);
    m_undoButton->setEnabled(false);
    m_undoButton->setToolTip("Ctrl+Z");
    m_undoButton->setStyleSheet(
        "QPushButton { padding: 5px 10px; border-radius: 3px; border: none; }"
        "QPushButton:enabled { background-color: #FF9800; color: white; }"
        "QPushButton:enabled:hover { background-color: #F57C00; }"
        "QPushButton:enabled:pressed { background-color: #E65100; }"
        "QPushButton:disabled { background-color: #E0E0E0; color: #9E9E9E; }");
    connect(m_undoButton, &QPushButton::clicked, this, &CustomGraphicsWidget::undo);

    // 重做按钮 - 可用时青色，表示"前进"操作
    m_redoButton = new QPushButton("重做", m_toolBar);
    m_redoButton->setFixedWidth(60);
    m_redoButton->setEnabled(false);
    m_redoButton->setToolTip("Ctrl+Y");
    m_redoButton->setStyleSheet(
        "QPushButton { padding: 5px 10px; border-radius: 3px; border: none; }"
        "QPushButton:enabled { background-color: #00BCD4; color: white; }"
        "QPushButton:enabled:hover { background-color: #00ACC1; }"
        "QPushButton:enabled:pressed { background-color: #0097A7; }"
        "QPushButton:disabled { background-color: #E0E0E0; color: #9E9E9E; }");
    connect(m_redoButton, &QPushButton::clicked, this, &CustomGraphicsWidget::redo);

    // 模式状态标签
    m_modeLabel = new QLabel("当前: 绘制模式 (按 D 切换)", m_toolBar);
    m_modeLabel->setStyleSheet("color: #333;");

    toolLayout->addWidget(m_modeButton);
    toolLayout->addWidget(m_undoButton);
    toolLayout->addWidget(m_redoButton);
    toolLayout->addWidget(m_modeLabel);
    toolLayout->addStretch();

    // 帮助提示
    QLabel *helpLabel = new QLabel("滚轮缩放 | 中键平移 | 左键绘制/选择 | Delete删除 | Esc取消", m_toolBar);
    helpLabel->setStyleSheet("color: #666; font-size: 11px;");
    toolLayout->addWidget(helpLabel);

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(m_toolBar);

    // 视图和属性面板并排
    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);
    contentLayout->addWidget(m_view, 1);

    // 属性面板和缩略图已在构造函数中提前创建，此处加入右边栏布局
    QWidget *rightSidebar = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightSidebar);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);
    rightLayout->addWidget(m_propertyPanel, 1);
    rightLayout->addWidget(m_minimapWidget);
    rightLayout->addStretch();

    contentLayout->addWidget(rightSidebar);

    mainLayout->addLayout(contentLayout);

    // 更新模式显示
    updateModeDisplay();
}

void CustomGraphicsWidget::updateModeDisplay()
{
    if (!m_drawHandler || !m_modeButton || !m_modeLabel) {
        return;
    }

    bool isDrawMode = m_drawHandler->isDrawingActive();

    m_modeButton->setChecked(isDrawMode);
    m_modeButton->setText(isDrawMode ? "绘制模式" : "选择模式");

    m_modeLabel->setText(isDrawMode ?
        "当前: 绘制模式 (按 D 切换)" :
        "当前: 选择模式 (按 D 切换)");

    // 发射信号，通知外部模式状态变化
    emit drawingActiveChanged(isDrawMode);

    // 确保视图有焦点以接收按键事件
    if (m_view) {
        m_view->setFocus();
    }
}
void CustomGraphicsWidget::addTestItems()
{
    // 方式1：使用场景背景图片功能（推荐）
    // 创建一个示例背景图片（实际使用时可从文件加载）
    QPixmap bgPixmap(600, 400);
    bgPixmap.fill(QColor(240, 240, 240));  // 浅灰色背景
    
    // 在背景上绘制一些纹理（模拟石材）
    QPainter painter(&bgPixmap);
    painter.setPen(QPen(QColor(200, 200, 200), 1));
    for (int i = 0; i < 600; i += 20) {
        painter.drawLine(i, 0, i, 400);
    }
    for (int j = 0; j < 400; j += 20) {
        painter.drawLine(0, j, 600, j);
    }
    painter.end();
    
    // 设置场景背景图片（自动设置边界约束和场景大小，并通过信号同步到 DrawHandler）
    m_scene->setBackgroundPixmap(bgPixmap);
    qDebug() << "设置背景图片，尺寸:" << bgPixmap.width() << "x" << bgPixmap.height();
    qDebug() << "边界约束:" << m_scene->boundaryConstraint();

    // 添加已有切割区域示例
    auto *cutArea1 = new LabeledRectItem(50, 50, 150, 100);
    cutArea1->setPen(QPen(QColor(0, 100, 200), 2));
    cutArea1->setBrush(QBrush(QColor(100, 149, 237, 80)));
    cutArea1->setFlag(QGraphicsItem::ItemIsSelectable);
    cutArea1->setFlag(QGraphicsItem::ItemIsMovable);
    cutArea1->setData(ShapeMeta::Id, ShapeMeta::nextId());
    cutArea1->setData(ShapeMeta::Category, "CutArea");
    cutArea1->setData(ShapeMeta::ShapeType, ShapeMeta::Rect);
    cutArea1->setData(ShapeMeta::Name, "切割区域_1");
    PropMap props1;
    props1["typeName"] = PropField("切割区域", "类型:", PropType::Text, true, false);
    props1["width"] = PropField(150.0, "宽度:", PropType::Number, true, true);
    props1["height"] = PropField(100.0, "高度:", PropType::Number, true, true);
    cutArea1->setData(ShapeMeta::Props, QVariant::fromValue(props1));
    cutArea1->setLabelText("切割区域_1");
    m_scene->addItem(cutArea1);

    auto *cutArea2 = new LabeledRectItem(250, 150, 200, 150);
    cutArea2->setPen(QPen(QColor(0, 100, 200), 2));
    cutArea2->setBrush(QBrush(QColor(100, 149, 237, 80)));
    cutArea2->setFlag(QGraphicsItem::ItemIsSelectable);
    cutArea2->setFlag(QGraphicsItem::ItemIsMovable);
    cutArea2->setData(ShapeMeta::Id, ShapeMeta::nextId());
    cutArea2->setData(ShapeMeta::Category, "CutArea");
    cutArea2->setData(ShapeMeta::ShapeType, ShapeMeta::Rect);
    cutArea2->setData(ShapeMeta::Name, "切割区域_2");
    PropMap props2;
    props2["typeName"] = PropField("切割区域", "类型:", PropType::Text, true, false);
    props2["width"] = PropField(200.0, "宽度:", PropType::Number, true, true);
    props2["height"] = PropField(150.0, "高度:", PropType::Number, true, true);
    cutArea2->setData(ShapeMeta::Props, QVariant::fromValue(props2));
    cutArea2->setLabelText("切割区域_2");
    m_scene->addItem(cutArea2);
}

void CustomGraphicsWidget::connectHandlers()
{
    // 连接绘制完成信号
    if (m_drawHandler) {
        connect(m_drawHandler, &DrawHandler::shapeCreated,
                this, [](QGraphicsItem* item, DrawHandler::ShapeType type, const QString& name) {
            Q_UNUSED(item)
            QString typeStr;
            switch (type) {
            case DrawHandler::ShapeType::Rect: typeStr = "矩形"; break;
            case DrawHandler::ShapeType::Line: typeStr = "线条"; break;
            default: typeStr = "未知"; break;
            }
            qDebug() << "绘制完成:" << name << "类型:" << typeStr;
        });

        // 连接模式切换信号，更新 UI 显示
        connect(m_drawHandler, &DrawHandler::drawingActiveChanged,
                this, [this](bool active) {
            Q_UNUSED(active)
            updateModeDisplay();
        });

        // 同步场景边界约束到 DrawHandler，限制绘制范围
        if (m_scene) {
            connect(m_scene, &CustomGraphicsScene::boundaryConstraintChanged,
                    m_drawHandler, &DrawHandler::setBoundaryConstraint);
            // 初始同步：将场景当前边界约束传递给 DrawHandler
            if (m_scene->hasBoundaryConstraint()) {
                m_drawHandler->setBoundaryConstraint(m_scene->boundaryConstraint());
            }
        }
    }

    // 连接背景加载信号
    if (m_backgroundHandler) {
        connect(m_backgroundHandler, &BackgroundHandler::backgroundLoaded,
                this, [](bool success) {
            qDebug() << "背景加载:" << (success ? "成功" : "失败");
        });
    }

    // 连接辅助线选中/拖动/取消选中信号，同步属性面板
    if (m_guideLineHandler) {
        connect(m_guideLineHandler, &GuideLineHandler::guideLineSelected,
                this, [this](int index, GuideLine::Type type, qreal position) {
            m_selectedGuideLineIndex = index;
            if (m_propertyPanel) {
                m_propertyPanel->updateFromGuideLine(type, position);
            }
        });

        connect(m_guideLineHandler, &GuideLineHandler::guideLineMoved,
                this, [this](int index, qreal position) {
            m_selectedGuideLineIndex = index;
            if (m_propertyPanel) {
                // 获取辅助线类型
                GuideLine gl = m_guideLineHandler->guideLine(index);
                m_propertyPanel->updateFromGuideLine(gl.type, position);
            }
        });

        connect(m_guideLineHandler, &GuideLineHandler::guideLineDeselected,
                this, [this]() {
            m_selectedGuideLineIndex = -1;
            if (m_propertyPanel) {
                m_propertyPanel->clearPanel();
            }
        });
    }

    // 连接场景的图元移动信号，支持移动撤销
    if (m_scene) {
        connect(m_scene, &CustomGraphicsScene::itemMoved,
                this, [this](QGraphicsItem *item, const QPointF &oldPos, const QPointF &newPos) {
            // 创建移动命令并推送到撤销栈
            MoveShapeCommand *cmd = new MoveShapeCommand(item, oldPos, newPos);
            m_view->undoStack()->push(cmd);
            // 不增加计数器，因为 push 会自动调用 redo
            // 同步更新属性面板
            onItemMovedForPanel(item, oldPos, newPos);
        });

        // 连接选择变化信号，更新属性面板
        connect(m_scene, &QGraphicsScene::selectionChanged,
                this, &CustomGraphicsWidget::onSelectionChanged);

        // 连接拖动过程信号，实时刷新属性面板
        connect(m_scene, &CustomGraphicsScene::itemMoving,
                this, [this](QGraphicsItem *item) {
            if (m_propertyPanel && item && item == m_scene->selectedItems().value(0)) {
                m_propertyPanel->updateFromItem(item);
            }
        });
    }

    // 连接属性面板信号（m_propertyPanel 已在构造函数中提前创建）
    if (m_propertyPanel) {
        connect(m_propertyPanel, &PropertyPanel::propertyChanged,
                this, &CustomGraphicsWidget::onPropertyChanged);

        // 连接属性面板的辅助线位置编辑信号
        if (m_guideLineHandler) {
            connect(m_propertyPanel, &PropertyPanel::guideLinePropertyChanged,
                    this, [this](qreal position) {
                if (m_selectedGuideLineIndex >= 0 && m_guideLineHandler) {
                    m_guideLineHandler->setGuideLinePosition(m_selectedGuideLineIndex, position);
                    if (m_view) {
                        m_view->viewport()->update();
                    }
                }
            });
        }
    }

    // 连接缩略图信号（m_minimapWidget 已在构造函数中提前创建）
    if (m_minimapWidget && m_view) {
        m_minimapWidget->setView(m_view);

        // 边界约束变化时同步缩略图概览范围
        if (m_scene) {
            connect(m_scene, &CustomGraphicsScene::boundaryConstraintChanged,
                    m_minimapWidget, &MinimapWidget::setOverviewRect);
            // 初始同步边界约束
            if (m_scene->hasBoundaryConstraint()) {
                m_minimapWidget->setOverviewRect(m_scene->boundaryConstraint());
            }
        }
    }
}

void CustomGraphicsWidget::updateUndoRedoState()
{
    if (m_view) {
        bool canUndo = m_view->canUndo();
        bool canRedo = m_view->canRedo();

        if (m_undoButton) {
            m_undoButton->setEnabled(canUndo);
        }
        if (m_redoButton) {
            m_redoButton->setEnabled(canRedo);
        }

        // 发射信号，通知外部按钮状态变化
        emit canUndoChanged(canUndo);
        emit canRedoChanged(canRedo);
    }
}

void CustomGraphicsWidget::onSelectionChanged()
{
    if (!m_propertyPanel || !m_scene) {
        return;
    }

    QList<QGraphicsItem*> selected = m_scene->selectedItems();
    if (selected.size() == 1) {
        m_propertyPanel->updateFromItem(selected.first());
    } else {
        m_propertyPanel->clearPanel();
    }
}

void CustomGraphicsWidget::onItemMovedForPanel(QGraphicsItem *item, const QPointF &oldPos, const QPointF &newPos)
{
    Q_UNUSED(oldPos)
    Q_UNUSED(newPos)
    if (m_propertyPanel && item && item == m_scene->selectedItems().value(0)) {
        m_propertyPanel->updateFromItem(item);
    }
}

void CustomGraphicsWidget::onPropertyChanged(QGraphicsItem *item, const QString &key, const QVariant &value)
{
    if (!item) {
        return;
    }

    qDebug() << "[CustomGraphicsWidget] onPropertyChanged: key=" << key << "value=" << value;

    bool needRefreshPanel = false;

    // 处理几何属性编辑，写回图元实际几何
    if (key == "x" || key == "y") {
        double newVal = value.toDouble();
        if (auto *rectItem = dynamic_cast<QGraphicsRectItem*>(item)) {
            QRectF r = rectItem->rect();
            QPointF visualPos = rectItem->pos() + r.topLeft();
            if (key == "x") {
                visualPos.setX(newVal);
            } else {
                visualPos.setY(newVal);
            }
            // 修正 pos：视觉位置 - rect 偏移
            rectItem->setPos(visualPos - r.topLeft());
        } else if (auto *lineItem = dynamic_cast<QGraphicsLineItem*>(item)) {
            QLineF l = lineItem->line();
            QPointF visualPos = lineItem->pos() + l.p1();
            if (key == "x") {
                visualPos.setX(newVal);
            } else {
                visualPos.setY(newVal);
            }
            // 修正 pos：视觉位置 - line.p1 偏移
            lineItem->setPos(visualPos - l.p1());
        }
        needRefreshPanel = true;
    } else if (key == "width" || key == "height") {
        if (auto *rectItem = dynamic_cast<QGraphicsRectItem*>(item)) {
            QRectF r = rectItem->rect();
            if (key == "width") {
                r.setWidth(value.toDouble());
            } else {
                r.setHeight(value.toDouble());
            }
            rectItem->setRect(r);
        }
        needRefreshPanel = true;
    } else if (key == "length") {
        if (auto *lineItem = dynamic_cast<QGraphicsLineItem*>(item)) {
            QLineF l = lineItem->line();
            double newLength = value.toDouble();
            if (newLength > 0 && l.length() > 0) {
                // 按方向缩放线条长度
                QPointF unitDir = (l.p2() - l.p1()) / l.length();
                l.setP2(l.p1() + unitDir * newLength);
                lineItem->setLine(l);
            }
        }
        needRefreshPanel = true;
    }

    // 刷新场景显示
    if (m_scene) {
        m_scene->update();
    }

    // 名称变更，更新图元标签
    if (key == "name") {
        if (m_propertyPanel) {
            m_propertyPanel->updateFromItem(item);
        }
    }

    // 几何属性变更后刷新面板（确保坐标补偿后显示正确）
    if (needRefreshPanel && m_propertyPanel) {
        m_propertyPanel->updateFromItem(item);
    }
}
