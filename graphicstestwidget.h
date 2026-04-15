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

/**
 * @brief 石材切割场景主窗口。
 *
 * 基于石材切割机可视化裁剪预览和编辑需求，集成：
 * - 高精度标尺（1mm = 10像素）
 * - 石板背景图片加载
 * - 矩形绘制与命名功能
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

private:
    /**
     * @brief 初始化石材切割场景。
     *
     * 创建视图和场景，加载场景配置中的 Handler。
     */
    void initStoneCuttingScene();

    /**
     * @brief 向场景中添加测试图元。
     */
    void addTestItems();

    /**
     * @brief 连接 Handler 信号槽。
     */
    void connectHandlers();

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
};
#endif // GRAPHICSTESTWIDGET_H
