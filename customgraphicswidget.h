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
class DrawHandler;
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
};
#endif // CUSTOMGRAPHICSWIDGET_H
