#include "PanHandler.h"

#include <QGraphicsView>
#include <QMouseEvent>
#include <QScrollBar>

PanHandler::PanHandler(int priority, QObject *parent)
    : AbstractInteractionHandler(priority, parent)
    , m_panButtons(Qt::MiddleButton | Qt::RightButton)
    , m_isPanning(false)
{
}

bool PanHandler::handleMousePress(QGraphicsView *view, QMouseEvent *event)
{
    if (m_panButtons.testFlag(event->button())) {
        m_isPanning = true;
        m_lastMousePos = event->pos();
        view->viewport()->setCursor(Qt::ClosedHandCursor);
        return true;
    }
    return false;
}

bool PanHandler::handleMouseMove(QGraphicsView *view, QMouseEvent *event)
{
    if (!m_isPanning) {
        return false;
    }

    const QPoint delta = event->pos() - m_lastMousePos;
    m_lastMousePos = event->pos();

    view->horizontalScrollBar()->setValue(
        view->horizontalScrollBar()->value() - delta.x());
    view->verticalScrollBar()->setValue(
        view->verticalScrollBar()->value() - delta.y());

    return true;
}

bool PanHandler::handleMouseRelease(QGraphicsView *view, QMouseEvent *event)
{
    if (!m_isPanning) {
        return false;
    }

    if (m_panButtons.testFlag(event->button())) {
        m_isPanning = false;
        view->viewport()->setCursor(Qt::ArrowCursor);
        return true;
    }
    return false;
}

QString PanHandler::handlerName() const
{
    return QStringLiteral("PanHandler");
}

void PanHandler::setPanButtons(Qt::MouseButtons buttons)
{
    m_panButtons = buttons;
}

Qt::MouseButtons PanHandler::panButtons() const
{
    return m_panButtons;
}
