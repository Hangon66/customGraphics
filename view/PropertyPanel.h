#ifndef PROPERTYPANEL_H
#define PROPERTYPANEL_H

#include <QWidget>
#include "ShapeMetadata.h"

class QLabel;
class QLineEdit;
class QDoubleSpinBox;
class QPushButton;
class QGraphicsItem;
class QVBoxLayout;
class QScrollArea;

/**
 * @brief 图元属性面板，动态显示并编辑选中图元的属性。
 *
 * 位于主窗口右侧的可折叠面板。通过遍历图元的 data(ShapeMeta::Props)
 * 属性表（PropMap）动态生成属旧行，无需硬编码类型信息。
 * 每个属性行包含显示名标签和编辑控件，根据 PropField 的 visible 和 editable
 * 决定是否显示和可编辑。
 */
class PropertyPanel : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数。
     *
     * @param parent 父 QWidget 对象。
     */
    explicit PropertyPanel(QWidget *parent = nullptr);

    /**
     * @brief 析构函数。
     */
    ~PropertyPanel() override;

    /**
     * @brief 更新面板显示指定图元的属性。
     *
     * 从图元的 data(Props) 读取 PropMap，遍历生成动态属性行。
     * 若 item 为 nullptr，则显示"未选中图元"提示。
     *
     * @param item 选中的图元，nullptr 表示无选中。
     */
    void updateFromItem(QGraphicsItem *item);

    /**
     * @brief 清除面板内容，显示未选中提示。
     */
    void clearPanel();

    /**
     * @brief 设置面板是否可见。
     *
     * @param visible true 显示面板；false 隐藏面板。
     */
    void setPanelVisible(bool visible);

    /**
     * @brief 获取面板是否可见。
     *
     * @return true 可见；false 隐藏。
     */
    bool isPanelVisible() const;

    /**
     * @brief 切换面板展开/折叠状态。
     */
    void togglePanel();

signals:
    /**
     * @brief 图元属性被编辑修改的信号。
     *
     * @param item 被修改的图元。
     * @param key 修改的属性键名。
     * @param value 修改后的值。
     */
    void propertyChanged(QGraphicsItem *item, const QString &key, const QVariant &value);

private slots:
    /**
     * @brief 名称编辑完成时的处理槽。
     */
    void onNameEditingFinished();

private:
    /**
     * @brief 初始化面板 UI 控件和布局。
     */
    void initUI();

    /**
     * @brief 根据属性表动态生成属性行。
     *
     * @param props 属性表。
     */
    void buildPropertyRows(const QMap<QString, PropField> &props);

    /**
     * @brief 清除所有动态属性行。
     */
    void clearPropertyRows();

    /**
     * @brief 仅更新已有控件的显示值，不重建布局。
     *
     * 用于拖动等频繁更新场景，避免销毁重建控件导致的闪烁和编辑中断。
     *
     * @param props 包含最新值的属性表。
     */
    void updatePropertyValues(const PropMap &props);

    /**
     * @brief 从图元实际几何刷新 PropMap 中的几何属性值。
     *
     * 对于矩形：x/y = pos() + rect().topLeft()，width/height = rect().size()。
     * 对于线条：x/y = pos() + line().p1()，length = line().length()。
     * 业务属性（如石板编号）不受影响。
     *
     * @param item 图元对象。
     * @param props 属性表引用，将被就地修改。
     */
    void refreshGeometryProps(QGraphicsItem *item, PropMap &props);

    /**
     * @brief 阻塞信号标志，防止程序性更新触发编辑信号。
     */
    bool m_updating;

    /**
     * @brief 当前关联的图元。
     */
    QGraphicsItem *m_currentItem;

    /**
     * @brief 标题标签。
     */
    QLabel *m_titleLabel;

    /**
     * @brief 名称输入框（始终显示）。
     */
    QLineEdit *m_nameEdit;

    /**
     * @brief 折叠/展开按钮。
     */
    QPushButton *m_toggleButton;

    /**
     * @brief 折叠/展开按钮容器，始终可见。
     */
    QWidget *m_toggleBar;

    /**
     * @brief 面板主体容器（标题+内容），折叠时隐藏。
     */
    QWidget *m_panelBody;

    /**
     * @brief 属性内容容器。
     */
    QWidget *m_contentWidget;

    /**
     * @brief 动态属性行的容器布局。
     */
    QVBoxLayout *m_propsLayout;

    /**
     * @brief 未选中提示标签。
     */
    QLabel *m_noSelectionLabel;

    /**
     * @brief 动态生成的属性行控件列表，用于清理。
     */
    QList<QWidget*> m_dynamicWidgets;

    /**
     * @brief 属性键到编辑控件的映射，用于高效刷新值。
     *
     * 键为属性名（如 "x"、"width"），值为 QDoubleSpinBox 或 QLineEdit 指针。
     */
    QMap<QString, QWidget*> m_propWidgets;

    /**
     * @brief 上次构建属性行时的键列表，用于判断是否需要重建。
     *
     * 若当前 PropMap 的键列表与 m_lastPropKeys 相同，则仅更新值；否则重建控件。
     */
    QStringList m_lastPropKeys;
};

#endif // PROPERTYPANEL_H
