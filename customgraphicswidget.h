#ifndef CUSTOMGRAPHICSWIDGET_H
#define CUSTOMGRAPHICSWIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class CustomGraphicsWidget;
}
QT_END_NAMESPACE

class CustomGraphicsView;
class CustomGraphicsScene;
class BackgroundHandler;
class RulerHandler;
class GuideLineHandler;
class DrawHandler;
class PropertyPanel;
class MinimapWidget;
class QGraphicsItem;
class QVariant;
class LabeledRectItem;
class QPushButton;
class QLabel;

/**
 * @brief 石材切割场景主窗口。
 *
 * 基于石材切割机可视化裁剪预览和编辑需求，集成：
 * - 高精度标尺（1mm = 10像素）
 * - 石板背景图片加载
 * - 矩形绘制与命名功能
 * - 绘制/选择模式切换
 */
class CustomGraphicsWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数，初始化石材切割场景窗口。
     *
     * @param parent 父 QWidget 对象。
     */
    explicit CustomGraphicsWidget(QWidget *parent = nullptr);
    ~CustomGraphicsWidget() override;

    /**
     * @brief 获取自定义图形视图实例。
     *
     * @return CustomGraphicsView 指针。
     */
    CustomGraphicsView* view() const { return m_view; }

    /**
     * @brief 获取自定义图形场景实例。
     *
     * @return CustomGraphicsScene 指针。
     */
    CustomGraphicsScene* scene() const { return m_scene; }

    /**
     * @brief 获取缩略图导航组件。
     *
     * 外部窗口可通过此方法获取缩略图，调用 setParent() 嵌入到外部窗口中显示。
     * 嵌入后缩略图会自动从当前布局中移除（Qt 的 reparent 机制）。
     *
     * @return MinimapWidget 指针。
     */
    MinimapWidget* minimapWidget() const { return m_minimapWidget; }

    // ========== 模式与操作公共接口 ==========

    /**
     * @brief 切换绘制/选择模式。
     *
     * 外部调用时与工具栏按钮等效，
     * 会在绘制模式和选择模式之间切换。
     */
    void toggleDrawSelectMode();

    /**
     * @brief 切换到绘制模式。
     *
     * 无论当前处于何种模式，均切换为绘制模式。
     */
    void setDrawMode();

    /**
     * @brief 切换到选择模式。
     *
     * 无论当前处于何种模式，均切换为选择模式。
     */
    void setSelectMode();

    /**
     * @brief 执行撤销操作。
     *
     * 撤销上一步操作，与点击撤销按钮等效。
     */
    void undo();

    /**
     * @brief 执行重做操作。
     *
     * 重做上一次撤销的操作，与点击重做按钮等效。
     */
    void redo();

    /**
     * @brief 获取当前是否为绘制模式。
     *
     * @return true 表示当前为绘制模式；false 表示为选择模式。
     */
    bool isDrawingActive() const;

    /**
     * @brief 设置工具栏是否可见。
     *
     * @param visible true 显示工具栏；false 隐藏工具栏。
     */
    void setToolBarVisible(bool visible);

    /**
     * @brief 获取工具栏是否可见。
     *
     * @return true 工具栏可见；false 工具栏隐藏。
     */
    bool isToolBarVisible() const;

signals:
    /**
     * @brief 撤销可用状态变化信号。
     *
     * 当撤销栈的 canUndo 状态发生变化时发出，
     * 外部可连接此信号自动更新撤销按钮状态。
     *
     * @param canUndo true 表示当前可以撤销；false 表示不能撤销。
     */
    void canUndoChanged(bool canUndo);

    /**
     * @brief 重做可用状态变化信号。
     *
     * 当撤销栈的 canRedo 状态发生变化时发出，
     * 外部可连接此信号自动更新重做按钮状态。
     *
     * @param canRedo true 表示当前可以重做；false 表示不能重做。
     */
    void canRedoChanged(bool canRedo);

    /**
     * @brief 绘制模式状态变化信号。
     *
     * 当绘制/选择模式发生切换时发出，
     * 外部可连接此信号同步模式按钮显示状态。
     *
     * @param active true 表示当前为绘制模式；false 表示为选择模式。
     */
    void drawingActiveChanged(bool active);

protected:
    /**
     * @brief 重写按键事件，支持快捷键切换模式。
     *
     * @param event 按键事件。
     */
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    /**
     * @brief 更新撤销/重做按钮状态。
     */
    void updateUndoRedoState();

    /**
     * @brief 场景选择变化时更新属性面板。
     */
    void onSelectionChanged();

    /**
     * @brief 图元移动后更新属性面板位置显示。
     *
     * @param item 被移动的图元。
     * @param oldPos 移动前的位置。
     * @param newPos 移动后的位置。
     */
    void onItemMovedForPanel(QGraphicsItem *item, const QPointF &oldPos, const QPointF &newPos);

    /**
     * @brief 属性面板编辑后刷新图元显示。
     *
     * @param item 被编辑的图元。
     * @param key 修改的属性键名。
     * @param value 修改后的值。
     */
    void onPropertyChanged(QGraphicsItem *item, const QString &key, const QVariant &value);

private:
    /**
     * @brief 初始化石材切割场景。
     */
    void initStoneCuttingScene();

    /**
     * @brief 初始化工具栏。
     */
    void initToolBar();

    /**
     * @brief 向场景中添加测试图元。
     */
    void addTestItems();

    /**
     * @brief 连接 Handler 信号槽。
     */
    void connectHandlers();

    /**
     * @brief 更新模式状态显示。
     */
    void updateModeDisplay();

    /**
     * @brief Qt Designer 生成的 UI 对象指针。
     */
    Ui::CustomGraphicsWidget *ui;

    /**
     * @brief 自定义图形场景实例。
     */
    CustomGraphicsScene *m_scene;

    /**
     * @brief 自定义图形视图实例。
     */
    CustomGraphicsView *m_view;

    /**
     * @brief 背景图片处理器。
     */
    BackgroundHandler *m_backgroundHandler;

    /**
     * @brief 标尺处理器。
     */
    RulerHandler *m_rulerHandler;

    /**
     * @brief 辅助线处理器。
     */
    GuideLineHandler *m_guideLineHandler;

    /**
     * @brief 绘制处理器。
     */
    DrawHandler *m_drawHandler;

    /**
     * @brief 模式切换按钮。
     */
    QPushButton *m_modeButton;

    /**
     * @brief 模式状态标签。
     */
    QLabel *m_modeLabel;

    /**
     * @brief 撤销按钮。
     */
    QPushButton *m_undoButton;

    /**
     * @brief 重做按钮。
     */
    QPushButton *m_redoButton;

    /**
     * @brief 工具栏容器。
     */
    QWidget *m_toolBar;

    /**
     * @brief 属性面板。
     */
    PropertyPanel *m_propertyPanel;

    /**
     * @brief 缩略图导航组件。
     */
    MinimapWidget *m_minimapWidget;

    /**
     * @brief 当前选中的辅助线索引，-1 表示未选中。
     */
    int m_selectedGuideLineIndex;
};
#endif // CUSTOMGRAPHICSWIDGET_H
