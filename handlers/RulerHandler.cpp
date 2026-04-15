#include "RulerHandler.h"

#include <QGraphicsView>
#include <QPainter>
#include <QFont>
#include <QtMath>

RulerHandler::RulerHandler(double unitPixel, const QString &unitName,
                           int priority, QObject *parent)
    : AbstractInteractionHandler(priority, parent)
    , m_unitPixel(unitPixel)
    , m_unitName(unitName)
    , m_rulerWidth(25)
    , m_backgroundColor(240, 240, 240)
    , m_tickColor(80, 80, 80)
    , m_position(RulerPosition::Both)
    , m_visible(true)
    , m_font("Arial", 8)
{
}

RulerHandler::~RulerHandler()
{
}

QString RulerHandler::handlerName() const
{
    return QStringLiteral("RulerHandler");
}

void RulerHandler::setUnitPixel(double unitPixel)
{
    if (unitPixel > 0 && !qFuzzyCompare(m_unitPixel, unitPixel)) {
        m_unitPixel = unitPixel;
        refresh();
    }
}

double RulerHandler::unitPixel() const
{
    return m_unitPixel;
}

void RulerHandler::setUnitName(const QString &name)
{
    if (m_unitName != name) {
        m_unitName = name;
        refresh();
    }
}

QString RulerHandler::unitName() const
{
    return m_unitName;
}

void RulerHandler::setRulerWidth(int width)
{
    if (width > 0 && m_rulerWidth != width) {
        m_rulerWidth = width;
        refresh();
    }
}

int RulerHandler::rulerWidth() const
{
    return m_rulerWidth;
}

void RulerHandler::setBackgroundColor(const QColor &color)
{
    m_backgroundColor = color;
    refresh();
}

QColor RulerHandler::backgroundColor() const
{
    return m_backgroundColor;
}

void RulerHandler::setTickColor(const QColor &color)
{
    m_tickColor = color;
    refresh();
}

QColor RulerHandler::tickColor() const
{
    return m_tickColor;
}

void RulerHandler::setRulerPosition(RulerPosition position)
{
    if (m_position != position) {
        m_position = position;
        refresh();
    }
}

RulerHandler::RulerPosition RulerHandler::rulerPosition() const
{
    return m_position;
}

void RulerHandler::setVisible(bool visible)
{
    if (m_visible != visible) {
        m_visible = visible;
        refresh();
    }
}

bool RulerHandler::isVisible() const
{
    return m_visible;
}

void RulerHandler::refresh()
{
    // 标尺在绘制时自动更新，无需额外操作
}

void RulerHandler::paint(QPainter *painter, QGraphicsView *view)
{
    if (!m_visible || !view) {
        return;
    }

    painter->save();
    painter->setFont(m_font);

    // 设置绘制模式，确保正确覆盖之前的内容
    painter->setCompositionMode(QPainter::CompositionMode_Source);

    QRect viewportRect = view->viewport()->rect();

    // 绘制顶部水平标尺
    if (m_position == RulerPosition::Top || m_position == RulerPosition::Both) {
        QRect topRect(0, 0, viewportRect.width(), m_rulerWidth);
        paintHorizontalRuler(painter, view, topRect);
    }

    // 绘制左侧垂直标尺
    if (m_position == RulerPosition::Left || m_position == RulerPosition::Both) {
        int leftOffset = (m_position == RulerPosition::Both) ? m_rulerWidth : 0;
        QRect leftRect(0, leftOffset, m_rulerWidth, viewportRect.height() - leftOffset);
        paintVerticalRuler(painter, view, leftRect);
    }

    // 绘制左上角原点区域
    if (m_position == RulerPosition::Both) {
        painter->fillRect(0, 0, m_rulerWidth, m_rulerWidth, m_backgroundColor);
        painter->setPen(m_tickColor);
        painter->drawText(QRect(0, 0, m_rulerWidth, m_rulerWidth),
                          Qt::AlignCenter, m_unitName);
    }

    painter->restore();
}

void RulerHandler::paintHorizontalRuler(QPainter *painter, QGraphicsView *view, const QRect &rect)
{
    // 绘制背景
    painter->fillRect(rect, m_backgroundColor);

    // 获取当前视图变换
    QTransform transform = view->transform();
    double scaleX = transform.m11();

    // 计算场景可见区域
    QRectF visibleRect = view->mapToScene(view->viewport()->rect()).boundingRect();

    // 每单位像素数（考虑缩放）
    double pixelsPerUnit = m_unitPixel * scaleX;

    // 计算刻度间隔
    double tickInterval = calculateTickInterval(pixelsPerUnit);

    // 起始刻度值
    double startUnit = qFloor(visibleRect.left() / m_unitPixel / tickInterval) * tickInterval;
    double endUnit = qCeil(visibleRect.right() / m_unitPixel / tickInterval) * tickInterval;

    painter->setPen(m_tickColor);

    // 绘制底部边线
    painter->drawLine(rect.left(), rect.bottom(), rect.right(), rect.bottom());

    // 绘制刻度
    for (double unit = startUnit; unit <= endUnit; unit += tickInterval / 5) {
        double sceneX = unit * m_unitPixel;
        int viewX = view->mapFromScene(QPointF(sceneX, 0)).x();

        if (viewX < rect.left() || viewX > rect.right()) {
            continue;
        }

        // 判断是主刻度还是次刻度
        bool isMajor = qFuzzyCompare(fmod(unit, tickInterval), 0.0) ||
                       qFuzzyCompare(fmod(unit, tickInterval), tickInterval);

        int tickHeight = isMajor ? rect.height() - 4 : rect.height() / 2;

        painter->drawLine(viewX, rect.bottom(), viewX, rect.bottom() - tickHeight);

        // 主刻度显示标签
        if (isMajor) {
            QString label = formatTickLabel(unit);
            painter->drawText(viewX + 2, rect.bottom() - tickHeight + 10, label);
        }
    }
}

void RulerHandler::paintVerticalRuler(QPainter *painter, QGraphicsView *view, const QRect &rect)
{
    // 绘制背景
    painter->fillRect(rect, m_backgroundColor);

    // 获取当前视图变换
    QTransform transform = view->transform();
    double scaleY = transform.m22();

    // 计算场景可见区域
    QRectF visibleRect = view->mapToScene(view->viewport()->rect()).boundingRect();

    // 每单位像素数（考虑缩放）
    double pixelsPerUnit = m_unitPixel * scaleY;

    // 计算刻度间隔
    double tickInterval = calculateTickInterval(pixelsPerUnit);

    // 起始刻度值
    double startUnit = qFloor(visibleRect.top() / m_unitPixel / tickInterval) * tickInterval;
    double endUnit = qCeil(visibleRect.bottom() / m_unitPixel / tickInterval) * tickInterval;

    painter->setPen(m_tickColor);

    // 绘制右侧边线
    painter->drawLine(rect.right(), rect.top(), rect.right(), rect.bottom());

    // 绘制刻度
    for (double unit = startUnit; unit <= endUnit; unit += tickInterval / 5) {
        double sceneY = unit * m_unitPixel;
        int viewY = view->mapFromScene(QPointF(0, sceneY)).y();

        if (viewY < rect.top() || viewY > rect.bottom()) {
            continue;
        }

        // 判断是主刻度还是次刻度
        bool isMajor = qFuzzyCompare(fmod(unit, tickInterval), 0.0) ||
                       qFuzzyCompare(fmod(unit, tickInterval), tickInterval);

        int tickWidth = isMajor ? rect.width() - 4 : rect.width() / 2;

        painter->drawLine(rect.right(), viewY, rect.right() - tickWidth, viewY);

        // 主刻度显示标签（垂直文字）
        if (isMajor) {
            QString label = formatTickLabel(unit);
            painter->save();
            painter->translate(rect.right() - tickWidth + 10, viewY - 2);
            painter->rotate(-90);
            painter->drawText(0, 0, label);
            painter->restore();
        }
    }
}

double RulerHandler::calculateTickInterval(double pixelsPerUnit) const
{
    // 目标：主刻度之间约 50-100 像素
    const double targetPixelGap = 75.0;

    // 计算基础间隔
    double baseInterval = targetPixelGap / pixelsPerUnit;

    // 规范化为 1, 2, 5 的倍数
    double magnitude = qPow(10, qFloor(qLn(baseInterval) / qLn(10)));
    double normalized = baseInterval / magnitude;

    if (normalized < 2) {
        return magnitude;
    } else if (normalized < 5) {
        return 2 * magnitude;
    } else {
        return 5 * magnitude;
    }
}

QString RulerHandler::formatTickLabel(double value) const
{
    if (qAbs(value) < 0.0001) {
        return QStringLiteral("0");
    }

    // 根据值大小选择合适的格式
    if (qAbs(value) >= 1000) {
        return QString::number(value, 'f', 0);
    } else if (qAbs(value) >= 1) {
        return QString::number(value, 'f', 1);
    } else {
        return QString::number(value, 'f', 2);
    }
}
