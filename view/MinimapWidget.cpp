#include "MinimapWidget.h"
#include "CustomGraphicsView.h"
#include "CustomGraphicsScene.h"
#include "../handlers/RulerHandler.h"

#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QEvent>
#include <QScrollBar>
#include <QMouseEvent>

MinimapWidget::MinimapWidget(QWidget *parent)
    : QWidget(parent)
    , m_view(nullptr)
    , m_isDragging(false)
{
    setMinimumSize(100, 80);
    setFixedWidth(220);
    setFixedHeight(160);
    setCursor(Qt::PointingHandCursor);
    setStyleSheet("background-color: #f5f5f5; border-top: 1px solid #ccc;");
}

MinimapWidget::~MinimapWidget()
{
}

void MinimapWidget::setView(CustomGraphicsView *view)
{
    // 移除旧视图的事件过滤器和信号连接
    if (m_view) {
        m_view->viewport()->removeEventFilter(this);
        disconnect(m_view, &CustomGraphicsView::viewportRegionChanged,
                   this, &MinimapWidget::onViewChanged);
        disconnect(m_view->horizontalScrollBar(), &QAbstractSlider::valueChanged,
                   this, &MinimapWidget::onViewChanged);
        disconnect(m_view->verticalScrollBar(), &QAbstractSlider::valueChanged,
                   this, &MinimapWidget::onViewChanged);
    }

    m_view = view;

    if (m_view) {
        // 视口区域变化信号（滚动时发出）
        connect(m_view, &CustomGraphicsView::viewportRegionChanged,
                this, &MinimapWidget::onViewChanged);

        // 滚动条值变化（覆盖滚动和缩放场景）
        connect(m_view->horizontalScrollBar(), &QAbstractSlider::valueChanged,
                this, &MinimapWidget::onViewChanged);
        connect(m_view->verticalScrollBar(), &QAbstractSlider::valueChanged,
                this, &MinimapWidget::onViewChanged);

        // 视口大小变化
        m_view->viewport()->installEventFilter(this);

        // 场景内容变化时重绘缩略图
        if (m_view->scene()) {
            connect(m_view->scene(), &QGraphicsScene::changed,
                    this, &MinimapWidget::onViewChanged);
        }
    }

    update();
}

void MinimapWidget::setOverviewRect(const QRectF &rect)
{
    m_overviewRect = rect;
    update();
}

QRectF MinimapWidget::overviewRect() const
{
    return m_overviewRect;
}

QWidget *MinimapWidget::overviewWidget()
{
    return this;
}

void MinimapWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    if (!m_view || !m_view->scene()) {
        painter.fillRect(rect(), QColor(245, 245, 245));
        painter.setPen(QColor(180, 180, 180));
        painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("无场景"));
        return;
    }

    // 确定概览范围：优先使用 m_overviewRect，否则使用 sceneRect
    QRectF sourceRect = m_overviewRect;
    if (sourceRect.isNull()) {
        sourceRect = m_view->scene()->sceneRect();
    }

    if (sourceRect.isNull() || sourceRect.isEmpty()) {
        painter.fillRect(rect(), QColor(245, 245, 245));
        return;
    }

    // 保持宽高比计算缩放
    QSizeF widgetSize = size();
    double scaleX = widgetSize.width() / sourceRect.width();
    double scaleY = widgetSize.height() / sourceRect.height();
    double scale = qMin(scaleX, scaleY);

    // 居中偏移
    double offsetX = (widgetSize.width() - sourceRect.width() * scale) / 2;
    double offsetY = (widgetSize.height() - sourceRect.height() * scale) / 2;

    QRectF targetRect(offsetX, offsetY,
                      sourceRect.width() * scale, sourceRect.height() * scale);

    // 先填充背景
    painter.fillRect(rect(), QColor(245, 245, 245));

    // 渲染场景缩略图
    m_view->scene()->render(&painter, targetRect, sourceRect);

    // 绘制视口矩形（扣除标尺遮挡区域）
    QRect viewportRect = m_view->viewport()->rect();

    // 获取标尺宽度，从视口矩形中扣除标尺遮挡部分
    RulerHandler *ruler = m_view->findHandler<RulerHandler>();
    int rulerW = (ruler ? ruler->rulerWidth() : 0);
    bool hasLeftRuler = ruler && (ruler->rulerPosition() == RulerHandler::RulerPosition::Left ||
                                  ruler->rulerPosition() == RulerHandler::RulerPosition::Both);
    bool hasBottomRuler = ruler && (ruler->rulerPosition() == RulerHandler::RulerPosition::Bottom ||
                                    ruler->rulerPosition() == RulerHandler::RulerPosition::Both);

    int leftOffset = hasLeftRuler ? rulerW : 0;
    int bottomOffset = hasBottomRuler ? rulerW : 0;
    QRectF effectiveViewportRect(leftOffset, 0,
                                  viewportRect.width() - leftOffset,
                                  viewportRect.height() - bottomOffset);

    QRectF viewportSceneRect = m_view->mapToScene(effectiveViewportRect.toRect()).boundingRect();

    // 将视口矩形映射到缩略图坐标
    double vpLeft = offsetX + (viewportSceneRect.left() - sourceRect.left()) * scale;
    double vpTop = offsetY + (viewportSceneRect.top() - sourceRect.top()) * scale;
    double vpWidth = viewportSceneRect.width() * scale;
    double vpHeight = viewportSceneRect.height() * scale;
    QRectF vpRect(vpLeft, vpTop, vpWidth, vpHeight);

    // 裁剪视口矩形到缩略图范围内
    vpRect = vpRect.intersected(targetRect.adjusted(0, 0, -1, -1));

    // 半透明橙色填充 + 橙色边框
    painter.setBrush(QColor(255, 165, 0, 60));
    painter.setPen(QPen(QColor(255, 140, 0), 1));
    painter.drawRect(vpRect);
}

void MinimapWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    update();
}

void MinimapWidget::onViewChanged()
{
    update();
}

bool MinimapWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (m_view && (obj == m_view->viewport() ||
                   obj == m_view->horizontalScrollBar() ||
                   obj == m_view->verticalScrollBar())) {
        // 监听视口 resize 和滚动条值变化
        if (event->type() == QEvent::Resize) {
            update();
        }
    }
    return QWidget::eventFilter(obj, event);
}

void MinimapWidget::navigateToViewPos(const QPoint &widgetPos)
{
    if (!m_view || !m_view->scene()) return;

    QRectF sourceRect = m_overviewRect.isNull()
        ? m_view->scene()->sceneRect() : m_overviewRect;
    if (sourceRect.isEmpty()) return;

    QSizeF widgetSize = size();
    double scaleX = widgetSize.width() / sourceRect.width();
    double scaleY = widgetSize.height() / sourceRect.height();
    double scale = qMin(scaleX, scaleY);
    double offsetX = (widgetSize.width() - sourceRect.width() * scale) / 2;
    double offsetY = (widgetSize.height() - sourceRect.height() * scale) / 2;

    double sceneX = (widgetPos.x() - offsetX) / scale + sourceRect.left();
    double sceneY = (widgetPos.y() - offsetY) / scale + sourceRect.top();

    m_view->centerOn(sceneX, sceneY);
}

void MinimapWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        return;
    }

    // 点击缩略图：先跳转视口，再进入拖拽模式
    navigateToViewPos(event->pos());
    m_isDragging = true;
}

void MinimapWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_isDragging) {
        return;
    }

    // 拖拽中：实时平移主视图
    navigateToViewPos(event->pos());
}

void MinimapWidget::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    m_isDragging = false;
}
