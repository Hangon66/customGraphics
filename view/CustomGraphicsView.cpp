#include "CustomGraphicsView.h"
#include "../handlers/IInteractionHandler.h"
#include "../handlers/ZoomHandler.h"
#include "../handlers/PanHandler.h"
#include "../handlers/RubberBandHandler.h"
#include "../handlers/RulerHandler.h"

#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QPainter>
#include <algorithm>

CustomGraphicsView::CustomGraphicsView(QWidget *parent)
    : QGraphicsView(parent)
    , m_handlersDirty(false)
    , m_activeMouseHandler(nullptr)
{
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setDragMode(QGraphicsView::NoDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);

    // 设置焦点策略，确保能够接收键盘事件
    setFocusPolicy(Qt::StrongFocus);
    setFocus();
}

CustomGraphicsView *CustomGraphicsView::createDefaultView(QWidget *parent)
{
    auto *view = new CustomGraphicsView(parent);
    view->addHandler(new ZoomHandler(100));
    view->addHandler(new PanHandler(50));
    view->addHandler(new RubberBandHandler(10));
    return view;
}

void CustomGraphicsView::addHandler(IInteractionHandler *handler)
{
    if (!handler || m_handlers.contains(handler)) {
        return;
    }
    m_handlers.append(handler);
    m_handlersDirty = true;

    // 如果 Handler 继承了 QObject 且无父对象，自动托管生命周期
    auto *obj = dynamic_cast<QObject *>(handler);
    if (obj && !obj->parent()) {
        obj->setParent(this);
    }
}

void CustomGraphicsView::removeHandler(IInteractionHandler *handler)
{
    m_handlers.removeOne(handler);
    if (m_activeMouseHandler == handler) {
        m_activeMouseHandler = nullptr;
    }
}

IInteractionHandler *CustomGraphicsView::findHandler(const QString &name) const
{
    for (IInteractionHandler *handler : m_handlers) {
        if (handler->handlerName() == name) {
            return handler;
        }
    }
    return nullptr;
}

QList<IInteractionHandler *> CustomGraphicsView::handlers() const
{
    return m_handlers;
}

void CustomGraphicsView::setHandlerEnabled(const QString &name, bool enabled)
{
    IInteractionHandler *handler = findHandler(name);
    if (handler) {
        handler->setEnabled(enabled);
    }
}

int CustomGraphicsView::deleteSelectedItems()
{
    if (!scene()) {
        return 0;
    }

    QList<QGraphicsItem*> selectedItems = scene()->selectedItems();
    int count = selectedItems.size();

    for (QGraphicsItem *item : selectedItems) {
        scene()->removeItem(item);
        delete item;
    }

    return count;
}

bool CustomGraphicsView::hasSelectedItems() const
{
    if (!scene()) {
        return false;
    }
    return !scene()->selectedItems().isEmpty();
}

void CustomGraphicsView::ensureHandlersSorted()
{
    if (m_handlersDirty) {
        std::stable_sort(m_handlers.begin(), m_handlers.end(),
                         [](const IInteractionHandler *a, const IInteractionHandler *b) {
                             return a->priority() > b->priority();
                         });
        m_handlersDirty = false;
    }
}

void CustomGraphicsView::mousePressEvent(QMouseEvent *event)
{
    ensureHandlersSorted();

    for (IInteractionHandler *handler : m_handlers) {
        if (!handler->isEnabled()) {
            continue;
        }
        if (handler->handleMousePress(this, event)) {
            m_activeMouseHandler = handler;
            event->accept();
            return;
        }
    }

    QGraphicsView::mousePressEvent(event);
}

void CustomGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    // 如果存在活跃处理器，优先发送给它
    if (m_activeMouseHandler && m_activeMouseHandler->isEnabled()) {
        if (m_activeMouseHandler->handleMouseMove(this, event)) {
            event->accept();
            return;
        }
    }

    ensureHandlersSorted();

    for (IInteractionHandler *handler : m_handlers) {
        if (!handler->isEnabled() || handler == m_activeMouseHandler) {
            continue;
        }
        if (handler->handleMouseMove(this, event)) {
            event->accept();
            return;
        }
    }

    QGraphicsView::mouseMoveEvent(event);
}

void CustomGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    // 如果存在活跃处理器，将 release 事件发送给它并清除活跃状态
    if (m_activeMouseHandler && m_activeMouseHandler->isEnabled()) {
        IInteractionHandler *active = m_activeMouseHandler;
        m_activeMouseHandler = nullptr;
        if (active->handleMouseRelease(this, event)) {
            event->accept();
            return;
        }
    } else {
        m_activeMouseHandler = nullptr;
    }

    ensureHandlersSorted();

    for (IInteractionHandler *handler : m_handlers) {
        if (!handler->isEnabled()) {
            continue;
        }
        if (handler->handleMouseRelease(this, event)) {
            event->accept();
            return;
        }
    }

    QGraphicsView::mouseReleaseEvent(event);
}

void CustomGraphicsView::wheelEvent(QWheelEvent *event)
{
    ensureHandlersSorted();

    for (IInteractionHandler *handler : m_handlers) {
        if (!handler->isEnabled()) {
            continue;
        }
        if (handler->handleWheel(this, event)) {
            event->accept();
            return;
        }
    }

    QGraphicsView::wheelEvent(event);
}

void CustomGraphicsView::keyPressEvent(QKeyEvent *event)
{
    // Delete 键删除选中图元
    if (event->key() == Qt::Key_Delete) {
        int deleted = deleteSelectedItems();
        if (deleted > 0) {
            event->accept();
            return;
        }
    }

    ensureHandlersSorted();

    for (IInteractionHandler *handler : m_handlers) {
        if (!handler->isEnabled()) {
            continue;
        }
        if (handler->handleKeyPress(this, event)) {
            event->accept();
            return;
        }
    }

    QGraphicsView::keyPressEvent(event);
}

void CustomGraphicsView::keyReleaseEvent(QKeyEvent *event)
{
    ensureHandlersSorted();

    for (IInteractionHandler *handler : m_handlers) {
        if (!handler->isEnabled()) {
            continue;
        }
        if (handler->handleKeyRelease(this, event)) {
            event->accept();
            return;
        }
    }

    QGraphicsView::keyReleaseEvent(event);
}

void CustomGraphicsView::paintEvent(QPaintEvent *event)
{
    // 先调用基类的绘制事件处理场景内容
    QGraphicsView::paintEvent(event);

    // 遍历 Handler，调用支持绘制的 Handler
    for (IInteractionHandler *handler : m_handlers) {
        if (!handler->isEnabled()) {
            continue;
        }
        // 检查是否是 RulerHandler 并调用其 paint 方法
        if (auto *rulerHandler = dynamic_cast<RulerHandler*>(handler)) {
            QPainter painter(viewport());
            painter.setRenderHint(QPainter::Antialiasing, false);
            rulerHandler->paint(&painter, this);
        }
    }
}

void CustomGraphicsView::scrollContentsBy(int dx, int dy)
{
    // 调用基类的滚动处理
    QGraphicsView::scrollContentsBy(dx, dy);

    // 强制更新 viewport 以重绘标尺
    // 标尺需要根据新的滚动位置重新绘制
    viewport()->update();
}
