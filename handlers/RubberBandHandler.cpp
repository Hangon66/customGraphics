#include "RubberBandHandler.h"

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QRubberBand>
#include <QPainterPath>

RubberBandHandler::RubberBandHandler(int priority, QObject *parent)
    : AbstractInteractionHandler(priority, parent)
    , m_selectButton(Qt::LeftButton)
    , m_selectionMode(Qt::IntersectsItemShape)
    , m_modifierForAdd(Qt::ControlModifier)
    , m_isSelecting(false)
    , m_rubberBand(nullptr)
{
}

bool RubberBandHandler::handleMousePress(QGraphicsView *view, QMouseEvent *event)
{
    if (event->button() != m_selectButton) {
        return false;
    }

    // 检查鼠标下方是否有图元，如果有则不启动框选，交给基类处理图元交互
    QGraphicsItem *itemUnderMouse = view->itemAt(event->pos());
    if (itemUnderMouse) {
        return false;
    }

    m_isSelecting = true;
    m_origin = event->pos();

    // 如果未按住追加修饰键，先清空已有选择
    if (!(event->modifiers() & m_modifierForAdd)) {
        if (view->scene()) {
            view->scene()->clearSelection();
        }
    }

    // 在视口上创建橡皮筋选框
    if (!m_rubberBand) {
        m_rubberBand = new QRubberBand(QRubberBand::Rectangle, view->viewport());
    }
    m_rubberBand->setGeometry(QRect(m_origin, QSize()));
    m_rubberBand->show();

    return true;
}

bool RubberBandHandler::handleMouseMove(QGraphicsView *view, QMouseEvent *event)
{
    Q_UNUSED(view)

    if (!m_isSelecting || !m_rubberBand) {
        return false;
    }

    m_rubberBand->setGeometry(QRect(m_origin, event->pos()).normalized());
    return true;
}

bool RubberBandHandler::handleMouseRelease(QGraphicsView *view, QMouseEvent *event)
{
    if (!m_isSelecting || event->button() != m_selectButton) {
        return false;
    }

    m_isSelecting = false;

    if (m_rubberBand) {
        // 获取最终选框区域
        const QRect rubberBandRect = m_rubberBand->geometry();

        // 隐藏并销毁橡皮筋
        m_rubberBand->hide();
        delete m_rubberBand;
        m_rubberBand = nullptr;

        // 将视口坐标映射到场景坐标并选中图元
        if (view->scene() && rubberBandRect.width() > 2 && rubberBandRect.height() > 2) {
            QPainterPath selectionPath;
            selectionPath.addPolygon(view->mapToScene(rubberBandRect));
            selectionPath.closeSubpath();

            // 确定选择操作类型：追加或替换
            Qt::ItemSelectionOperation operation = Qt::ReplaceSelection;
            if (event->modifiers() & m_modifierForAdd) {
                operation = Qt::AddToSelection;
            }

            view->scene()->setSelectionArea(selectionPath, operation,
                                            m_selectionMode,
                                            view->viewportTransform());
        }
    }

    return true;
}

QString RubberBandHandler::handlerName() const
{
    return QStringLiteral("RubberBandHandler");
}

void RubberBandHandler::setSelectButton(Qt::MouseButton button)
{
    m_selectButton = button;
}

void RubberBandHandler::setSelectionMode(Qt::ItemSelectionMode mode)
{
    m_selectionMode = mode;
}

void RubberBandHandler::setAddModifier(Qt::KeyboardModifier modifier)
{
    m_modifierForAdd = modifier;
}
