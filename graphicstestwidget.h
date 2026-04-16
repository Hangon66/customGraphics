#ifndef GRAPHICSTESTWIDGET_H
#define GRAPHICSTESTWIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class GraphicsTestWidget;
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
class GraphicsTestWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数，初始化石材切割场景窗口。
     *
     * @param parent 父 QWidget 对象。
     */
    explicit GraphicsTestWidget(QWidget *parent = nullptr);
    ~GraphicsTestWidget() override;

protected:
    /**
     * @brief 重写按键事件，支持快捷键切换模式。
     *
     * @param event 按键事件。
     */
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    /**
     * @brief 切换绘制/选择模式。
     */
    void toggleDrawSelectMode();

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
    Ui::GraphicsTestWidget *ui;

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
};
#endif // GRAPHICSTESTWIDGET_H
