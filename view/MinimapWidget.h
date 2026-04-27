#ifndef MINIMAPWIDGET_H
#define MINIMAPWIDGET_H

#include <QWidget>
#include <QRectF>

class CustomGraphicsView;

/**
 * @brief 缩略图导航组件，显示场景全局概览和当前视口位置。
 *
 * 以缩放方式渲染场景内容，叠加半透明视口矩形指示当前可见区域。
 * 默认概览范围为场景的边界约束区域，支持外部设置概览范围。
 * 可被外部窗口获取并 reparent 到外部窗口中显示。
 */
class MinimapWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数。
     *
     * @param parent 父 QWidget 对象。
     */
    explicit MinimapWidget(QWidget *parent = nullptr);

    /**
     * @brief 析构函数。
     */
    ~MinimapWidget() override;

    /**
     * @brief 设置关联的图形视图。
     *
     * 缩略图通过该视图获取场景和视口信息。
     *
     * @param view 关联的 CustomGraphicsView 指针。
     */
    void setView(CustomGraphicsView *view);

    /**
     * @brief 设置概览显示的场景范围。
     *
     * 缩略图将渲染此范围内的场景内容，默认跟随场景边界约束。
     *
     * @param rect 概览范围（场景坐标）；空矩形表示使用场景 sceneRect。
     */
    void setOverviewRect(const QRectF &rect);

    /**
     * @brief 获取当前概览显示的场景范围。
     *
     * @return 概览范围矩形（场景坐标）。
     */
    QRectF overviewRect() const;

    /**
     * @brief 获取缩略图组件的 Widget 指针，供外部获取并 reparent。
     *
     * @return 自身 QWidget 指针。
     */
    QWidget *overviewWidget();

protected:
    /**
     * @brief 重写绘制事件，渲染场景缩略图和视口矩形。
     *
     * @param event 绘制事件对象。
     */
    void paintEvent(QPaintEvent *event) override;

    /**
     * @brief 重写大小变化事件，触发重绘。
     *
     * @param event 大小变化事件对象。
     */
    void resizeEvent(QResizeEvent *event) override;

    /**
     * @brief 重写鼠标按下事件，实现点击导航和拖拽开始。
     *
     * @param event 鼠标事件对象。
     */
    void mousePressEvent(QMouseEvent *event) override;

    /**
     * @brief 重写鼠标移动事件，实现拖拽平移。
     *
     * @param event 鼠标事件对象。
     */
    void mouseMoveEvent(QMouseEvent *event) override;

    /**
     * @brief 重写鼠标释放事件，结束拖拽。
     *
     * @param event 鼠标事件对象。
     */
    void mouseReleaseEvent(QMouseEvent *event) override;

    /**
     * @brief 重写事件过滤器，监听关联视图的事件。
     *
     * @param obj 事件接收对象。
     * @param event 事件对象。
     * @return false 表示不拦截事件，继续传递。
     */
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    /**
     * @brief 视口变化时触发重绘。
     *
     * 连接到视图的滚动、缩放等变化信号。
     */
    void onViewChanged();

private:
    /**
     * @brief 关联的图形视图。
     */
    CustomGraphicsView *m_view;

    /**
     * @brief 概览显示的场景范围（场景坐标）。
     *
     * 空矩形表示使用场景的 sceneRect。
     */
    QRectF m_overviewRect;

    /**
     * @brief 是否正在拖拽视口矩形。
     */
    bool m_isDragging;

    /**
     * @brief 将缩略图坐标转换为主视图中心场景坐标并跳转。
     *
     * @param widgetPos 缩略图上的像素坐标。
     */
    void navigateToViewPos(const QPoint &widgetPos);
};

#endif // MINIMAPWIDGET_H
