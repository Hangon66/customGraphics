#ifndef IINTERACTIONHANDLER_H
#define IINTERACTIONHANDLER_H

#include <QObject>
#include <QString>

class QGraphicsView;
class QMouseEvent;
class QWheelEvent;
class QKeyEvent;

/**
 * @brief 交互处理器纯虚接口。
 *
 * 定义了所有交互处理器必须遵守的事件处理协议。
 * 每个 Handler 负责处理一种特定的交互行为（如缩放、平移、框选等），
 * 由 CustomGraphicsView 按优先级顺序分发事件。
 *
 * 事件处理方法返回 true 表示该事件已被消费，后续 Handler 不再处理；
 * 返回 false 表示未处理，事件继续传递给下一个 Handler。
 */
class IInteractionHandler
{
public:
    virtual ~IInteractionHandler() = default;

    /**
     * @brief 处理鼠标按下事件。
     *
     * @param view 产生事件的 QGraphicsView 实例。
     * @param event 鼠标按下事件对象。
     * @return true 表示事件已被消费；false 表示未处理。
     */
    virtual bool handleMousePress(QGraphicsView *view, QMouseEvent *event) = 0;

    /**
     * @brief 处理鼠标移动事件。
     *
     * @param view 产生事件的 QGraphicsView 实例。
     * @param event 鼠标移动事件对象。
     * @return true 表示事件已被消费；false 表示未处理。
     */
    virtual bool handleMouseMove(QGraphicsView *view, QMouseEvent *event) = 0;

    /**
     * @brief 处理鼠标释放事件。
     *
     * @param view 产生事件的 QGraphicsView 实例。
     * @param event 鼠标释放事件对象。
     * @return true 表示事件已被消费；false 表示未处理。
     */
    virtual bool handleMouseRelease(QGraphicsView *view, QMouseEvent *event) = 0;

    /**
     * @brief 处理鼠标滚轮事件。
     *
     * @param view 产生事件的 QGraphicsView 实例。
     * @param event 滚轮事件对象。
     * @return true 表示事件已被消费；false 表示未处理。
     */
    virtual bool handleWheel(QGraphicsView *view, QWheelEvent *event) = 0;

    /**
     * @brief 处理键盘按下事件。
     *
     * @param view 产生事件的 QGraphicsView 实例。
     * @param event 键盘按下事件对象。
     * @return true 表示事件已被消费；false 表示未处理。
     */
    virtual bool handleKeyPress(QGraphicsView *view, QKeyEvent *event) = 0;

    /**
     * @brief 处理键盘释放事件。
     *
     * @param view 产生事件的 QGraphicsView 实例。
     * @param event 键盘释放事件对象。
     * @return true 表示事件已被消费；false 表示未处理。
     */
    virtual bool handleKeyRelease(QGraphicsView *view, QKeyEvent *event) = 0;

    /**
     * @brief 获取处理器的优先级。
     *
     * 数值越大表示优先级越高，事件分发时越先被调用。
     *
     * @return 处理器的优先级数值。
     */
    virtual int priority() const = 0;

    /**
     * @brief 获取处理器是否处于启用状态。
     *
     * @return true 表示已启用；false 表示已禁用。
     */
    virtual bool isEnabled() const = 0;

    /**
     * @brief 设置处理器的启用/禁用状态。
     *
     * 禁用后的处理器在事件分发时会被跳过，但仍保留在处理器列表中。
     *
     * @param enabled true 启用；false 禁用。
     */
    virtual void setEnabled(bool enabled) = 0;

    /**
     * @brief 获取处理器的唯一名称标识。
     *
     * 用于调试、日志输出以及按名称查找处理器。
     *
     * @return 处理器的名称字符串。
     */
    virtual QString handlerName() const = 0;
};

/**
 * @brief 交互处理器抽象基类，提供 IInteractionHandler 的默认实现。
 *
 * 继承自 QObject 以利用 Qt 对象树的生命周期管理。
 * 所有事件处理方法默认返回 false（不消费事件），
 * 子类只需重写关心的事件处理方法即可。
 * 内建启用状态和优先级管理。
 */
class AbstractInteractionHandler : public QObject, public IInteractionHandler
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数。
     *
     * @param priority 处理器优先级，数值越大越先处理。
     * @param parent 父 QObject 对象，用于生命周期管理。
     */
    explicit AbstractInteractionHandler(int priority = 0, QObject *parent = nullptr)
        : QObject(parent)
        , m_priority(priority)
        , m_enabled(true)
    {
    }

    /**
     * @brief 处理鼠标按下事件，默认不消费。
     *
     * @param view 产生事件的 QGraphicsView 实例。
     * @param event 鼠标按下事件对象。
     * @return 始终返回 false。
     */
    bool handleMousePress(QGraphicsView *view, QMouseEvent *event) override
    {
        Q_UNUSED(view)
        Q_UNUSED(event)
        return false;
    }

    /**
     * @brief 处理鼠标移动事件，默认不消费。
     *
     * @param view 产生事件的 QGraphicsView 实例。
     * @param event 鼠标移动事件对象。
     * @return 始终返回 false。
     */
    bool handleMouseMove(QGraphicsView *view, QMouseEvent *event) override
    {
        Q_UNUSED(view)
        Q_UNUSED(event)
        return false;
    }

    /**
     * @brief 处理鼠标释放事件，默认不消费。
     *
     * @param view 产生事件的 QGraphicsView 实例。
     * @param event 鼠标释放事件对象。
     * @return 始终返回 false。
     */
    bool handleMouseRelease(QGraphicsView *view, QMouseEvent *event) override
    {
        Q_UNUSED(view)
        Q_UNUSED(event)
        return false;
    }

    /**
     * @brief 处理鼠标滚轮事件，默认不消费。
     *
     * @param view 产生事件的 QGraphicsView 实例。
     * @param event 滚轮事件对象。
     * @return 始终返回 false。
     */
    bool handleWheel(QGraphicsView *view, QWheelEvent *event) override
    {
        Q_UNUSED(view)
        Q_UNUSED(event)
        return false;
    }

    /**
     * @brief 处理键盘按下事件，默认不消费。
     *
     * @param view 产生事件的 QGraphicsView 实例。
     * @param event 键盘按下事件对象。
     * @return 始终返回 false。
     */
    bool handleKeyPress(QGraphicsView *view, QKeyEvent *event) override
    {
        Q_UNUSED(view)
        Q_UNUSED(event)
        return false;
    }

    /**
     * @brief 处理键盘释放事件，默认不消费。
     *
     * @param view 产生事件的 QGraphicsView 实例。
     * @param event 键盘释放事件对象。
     * @return 始终返回 false。
     */
    bool handleKeyRelease(QGraphicsView *view, QKeyEvent *event) override
    {
        Q_UNUSED(view)
        Q_UNUSED(event)
        return false;
    }

    /**
     * @brief 获取处理器的优先级。
     *
     * @return 构造时指定的优先级数值。
     */
    int priority() const override { return m_priority; }

    /**
     * @brief 获取处理器是否处于启用状态。
     *
     * @return true 表示已启用；false 表示已禁用。
     */
    bool isEnabled() const override { return m_enabled; }

    /**
     * @brief 设置处理器的启用/禁用状态。
     *
     * @param enabled true 启用；false 禁用。
     */
    void setEnabled(bool enabled) override { m_enabled = enabled; }

protected:
    /**
     * @brief 处理器优先级，数值越大越先被调用。
     */
    int m_priority;

    /**
     * @brief 处理器启用状态标志位。
     *
     * true 表示启用，事件分发时会调用该处理器；
     * false 表示禁用，事件分发时会跳过该处理器。
     */
    bool m_enabled;
};

#endif // IINTERACTIONHANDLER_H
