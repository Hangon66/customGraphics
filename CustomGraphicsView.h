#ifndef CUSTOMGRAPHICSVIEW_H
#define CUSTOMGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QList>

class IInteractionHandler;

/**
 * @brief 自定义图形视图类，作为交互处理器（Handler）的容器和事件分发中枢。
 *
 * 继承自 QGraphicsView，通过 Handler 组合模式管理多种交互行为。
 * 重写鼠标、滚轮、键盘等事件处理函数，按优先级将事件分发给已注册的 Handler。
 * 支持 Handler 的动态注册、注销、启用/禁用，以及按名称或类型查找。
 *
 * 设计为可跨项目复用的基础组件，使用时通过 addHandler() 注册所需的交互处理器，
 * 或调用 createDefaultView() 工厂方法快速创建带有默认 Handler 的视图。
 */
class CustomGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数，初始化视图并设置默认渲染参数。
     *
     * 默认启用抗锯齿、平滑像素变换，禁用内置拖拽模式（由 Handler 接管），
     * 设置缩放锚点为鼠标位置，使用智能视口更新模式。
     *
     * @param parent 父 QWidget 对象。
     */
    explicit CustomGraphicsView(QWidget *parent = nullptr);

    /**
     * @brief 创建带有默认 Handler 的视图实例（工厂方法）。
     *
     * 自动注册 ZoomHandler（优先级 100）、PanHandler（优先级 50）
     * 和 RubberBandHandler（优先级 10）三个默认交互处理器。
     *
     * @param parent 父 QWidget 对象。
     * @return 已注册默认 Handler 的 CustomGraphicsView 实例指针。
     */
    static CustomGraphicsView *createDefaultView(QWidget *parent = nullptr);

    /**
     * @brief 注册一个交互处理器。
     *
     * 将 Handler 添加到处理器列表中。如果 Handler 继承了 QObject 且无父对象，
     * 则自动将当前 View 设置为其父对象以托管生命周期。
     * 添加后会标记需要重新排序。
     *
     * @param handler 要注册的交互处理器指针，不可为 nullptr。
     */
    void addHandler(IInteractionHandler *handler);

    /**
     * @brief 注销一个交互处理器。
     *
     * 从处理器列表中移除指定 Handler，不会 delete 该对象。
     * 如果被移除的 Handler 是当前活跃的鼠标处理器，会同时清除活跃状态。
     *
     * @param handler 要注销的交互处理器指针。
     */
    void removeHandler(IInteractionHandler *handler);

    /**
     * @brief 按名称查找交互处理器。
     *
     * @param name 处理器的名称标识（由 handlerName() 返回）。
     * @return 匹配的处理器指针；未找到时返回 nullptr。
     */
    IInteractionHandler *findHandler(const QString &name) const;

    /**
     * @brief 按类型查找交互处理器（模板方法）。
     *
     * 使用 dynamic_cast 进行类型匹配，返回第一个匹配的 Handler。
     *
     * @tparam T 目标 Handler 类型。
     * @return 匹配的处理器指针；未找到时返回 nullptr。
     */
    template<typename T>
    T *findHandler() const
    {
        for (IInteractionHandler *handler : m_handlers) {
            T *result = dynamic_cast<T *>(handler);
            if (result) {
                return result;
            }
        }
        return nullptr;
    }

    /**
     * @brief 获取所有已注册的交互处理器列表。
     *
     * @return 处理器指针列表（只读副本）。
     */
    QList<IInteractionHandler *> handlers() const;

    /**
     * @brief 按名称设置处理器的启用/禁用状态。
     *
     * @param name 处理器的名称标识。
     * @param enabled true 启用；false 禁用。
     */
    void setHandlerEnabled(const QString &name, bool enabled);

protected:
    /**
     * @brief 处理鼠标按下事件并分发给 Handler。
     *
     * 按优先级遍历已启用的 Handler，第一个消费事件的 Handler 将被设为活跃鼠标处理器，
     * 后续的 mouseMove 和 mouseRelease 事件将优先发送给该 Handler。
     * 若所有 Handler 均未消费，则调用基类默认行为。
     *
     * @param event 鼠标按下事件对象。
     */
    void mousePressEvent(QMouseEvent *event) override;

    /**
     * @brief 处理鼠标移动事件并分发给 Handler。
     *
     * 如果存在活跃鼠标处理器，优先将事件发送给它；
     * 否则按优先级遍历已启用的 Handler。
     * 若所有 Handler 均未消费，则调用基类默认行为。
     *
     * @param event 鼠标移动事件对象。
     */
    void mouseMoveEvent(QMouseEvent *event) override;

    /**
     * @brief 处理鼠标释放事件并分发给 Handler。
     *
     * 如果存在活跃鼠标处理器，将事件发送给它并在处理完成后清除活跃状态。
     * 否则按优先级遍历已启用的 Handler。
     * 若所有 Handler 均未消费，则调用基类默认行为。
     *
     * @param event 鼠标释放事件对象。
     */
    void mouseReleaseEvent(QMouseEvent *event) override;

    /**
     * @brief 处理鼠标滚轮事件并分发给 Handler。
     *
     * 按优先级遍历已启用的 Handler。
     * 若所有 Handler 均未消费，则调用基类默认行为。
     *
     * @param event 滚轮事件对象。
     */
    void wheelEvent(QWheelEvent *event) override;

    /**
     * @brief 处理键盘按下事件并分发给 Handler。
     *
     * 按优先级遍历已启用的 Handler。
     * 若所有 Handler 均未消费，则调用基类默认行为。
     *
     * @param event 键盘按下事件对象。
     */
    void keyPressEvent(QKeyEvent *event) override;

    /**
     * @brief 处理键盘释放事件并分发给 Handler。
     *
     * 按优先级遍历已启用的 Handler。
     * 若所有 Handler 均未消费，则调用基类默认行为。
     *
     * @param event 键盘释放事件对象。
     */
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    /**
     * @brief 确保处理器列表按优先级降序排列。
     *
     * 仅在 m_handlersDirty 为 true 时执行排序（延迟排序优化）。
     */
    void ensureHandlersSorted();

    /**
     * @brief 已注册的交互处理器列表，按优先级降序排列。
     */
    QList<IInteractionHandler *> m_handlers;

    /**
     * @brief 标记处理器列表是否需要重新排序。
     *
     * 在 addHandler() 时置为 true，下次事件分发前执行排序。
     */
    bool m_handlersDirty;

    /**
     * @brief 当前持有鼠标按下事件序列的活跃处理器。
     *
     * 在 mousePressEvent 中设置，保证 press-move-release 三连事件
     * 发送给同一个 Handler，防止中途因优先级变化导致事件序列断裂。
     * mouseRelease 处理完毕后重置为 nullptr。
     */
    IInteractionHandler *m_activeMouseHandler;
};

#endif // CUSTOMGRAPHICSVIEW_H
