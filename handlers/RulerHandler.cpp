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
    , m_decimalPrecision(0)
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

void RulerHandler::setDecimalPrecision(int precision)
{
    if (precision >= 0 && precision != m_decimalPrecision) {
        m_decimalPrecision = precision;
    }
}

int RulerHandler::decimalPrecision() const
{
    return m_decimalPrecision;
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

    // 绘制底部水平标尺
    if (m_position == RulerPosition::Bottom || m_position == RulerPosition::Both) {
        QRect bottomRect(0, viewportRect.height() - m_rulerWidth, viewportRect.width(), m_rulerWidth);
        paintHorizontalRuler(painter, view, bottomRect);
    }

    // 绘制顶部水平标尺
    if (m_position == RulerPosition::Top) {
        QRect topRect(0, 0, viewportRect.width(), m_rulerWidth);
        paintHorizontalRuler(painter, view, topRect);
    }

    // 绘制左侧垂直标尺
    if (m_position == RulerPosition::Left || m_position == RulerPosition::Both) {
        int bottomOffset = (m_position == RulerPosition::Both) ? m_rulerWidth : 0;
        QRect leftRect(0, 0, m_rulerWidth, viewportRect.height() - bottomOffset);
        paintVerticalRuler(painter, view, leftRect);
    }

    // 绘制左下角原点区域
    if (m_position == RulerPosition::Both) {
        int cornerY = viewportRect.height() - m_rulerWidth;
        painter->fillRect(0, cornerY, m_rulerWidth, m_rulerWidth, m_backgroundColor);
        painter->setPen(m_tickColor);
        painter->drawText(QRect(0, cornerY, m_rulerWidth, m_rulerWidth),
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

    // 防止极小间隔导致的性能问题
    if (tickInterval < 0.001) {
        tickInterval = 0.001;
    }

    // 起始刻度值
    double startUnit = qFloor(visibleRect.left() / m_unitPixel / tickInterval) * tickInterval;
    double endUnit = qCeil(visibleRect.right() / m_unitPixel / tickInterval) * tickInterval;

    painter->setPen(m_tickColor);

    bool isAtBottom = (m_position == RulerPosition::Bottom || m_position == RulerPosition::Both);

    // 绘制分隔线：底部标尺在顶部画分隔线，顶部标尺在底部画分隔线
    if (isAtBottom) {
        painter->drawLine(rect.left(), rect.top(), rect.right(), rect.top());
    } else {
        painter->drawLine(rect.left(), rect.bottom(), rect.right(), rect.bottom());
    }

    // 绘制刻度
    double step = tickInterval / 5.0;
    int maxTicks = 1000;  // 防止过多刻度导致的性能问题
    int tickCount = 0;

    for (double unit = startUnit; unit <= endUnit && tickCount < maxTicks; unit += step) {
        tickCount++;

        double sceneX = unit * m_unitPixel;
        int viewX = view->mapFromScene(QPointF(sceneX, 0)).x();

        if (viewX < rect.left() || viewX > rect.right()) {
            continue;
        }

        // 判断是主刻度还是次刻度
        // 使用整数倍判断避免浮点精度问题
        double ratio = unit / tickInterval;
        int ratioInt = qRound(ratio);
        // 使用相对容差，对大数值更友好
        double epsilon = qMax(1e-9, qAbs(ratio) * 1e-9);
        bool isMajor = qAbs(ratio - ratioInt) < epsilon;

        int tickHeight = isMajor ? rect.height() - 4 : rect.height() / 2;

        if (isAtBottom) {
            // 底部标尺：刻度从顶边向下延伸
            painter->drawLine(viewX, rect.top(), viewX, rect.top() + tickHeight);
            if (isMajor) {
                QString label = formatTickLabel(unit);
                painter->drawText(viewX + 2, rect.top() + tickHeight - 2, label);
            }
        } else {
            // 顶部标尺：刻度从底边向上延伸
            painter->drawLine(viewX, rect.bottom(), viewX, rect.bottom() - tickHeight);
            if (isMajor) {
                QString label = formatTickLabel(unit);
                painter->drawText(viewX + 2, rect.bottom() - tickHeight + 10, label);
            }
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

    // 防止极小间隔导致的性能问题
    if (tickInterval < 0.001) {
        tickInterval = 0.001;
    }

    // 起始刻度值
    double startUnit = qFloor(visibleRect.top() / m_unitPixel / tickInterval) * tickInterval;
    double endUnit = qCeil(visibleRect.bottom() / m_unitPixel / tickInterval) * tickInterval;

    painter->setPen(m_tickColor);

    // 绘制右侧边线
    painter->drawLine(rect.right(), rect.top(), rect.right(), rect.bottom());

    // 绘制刻度
    double step = tickInterval / 5.0;
    int maxTicks = 1000;  // 防止过多刻度导致的性能问题
    int tickCount = 0;

    for (double unit = startUnit; unit <= endUnit && tickCount < maxTicks; unit += step) {
        tickCount++;

        double sceneY = unit * m_unitPixel;
        int viewY = view->mapFromScene(QPointF(0, sceneY)).y();

        if (viewY < rect.top() || viewY > rect.bottom()) {
            continue;
        }

        // 判断是主刻度还是次刻度
        // 使用整数倍判断避免浮点精度问题
        double ratioY = unit / tickInterval;
        int ratioIntY = qRound(ratioY);
        // 使用相对容差，对大数值更友好
        double epsilon = qMax(1e-9, qAbs(ratioY) * 1e-9);
        bool isMajor = qAbs(ratioY - ratioIntY) < epsilon;

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
    // 防止除零或极小值
    if (pixelsPerUnit <= 0.0001) {
        return 1.0;
    }

    // 目标：主刻度之间约 50-100 像素
    const double targetPixelGap = 75.0;

    // 计算基础间隔
    double baseInterval = targetPixelGap / pixelsPerUnit;

    // 防止极小值导致的计算问题
    if (baseInterval < 0.001) {
        return 0.001;  // 最小刻度间隔 0.001
    }

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
    // 处理极大或极小的值
    if (!qIsFinite(value)) {
        return QStringLiteral("0");
    }

    double absValue = qAbs(value);

    if (absValue < 0.0001) {
        return QStringLiteral("0");
    }

    // 根据值大小选择合适的格式，小数精度由 m_decimalPrecision 控制
    if (absValue >= 1000000) {
        return QString::number(value, 'e', 2);  // 科学计数法
    } else if (absValue >= 1000) {
        return QString::number(value, 'f', 0);
    } else {
        return QString::number(value, 'f', m_decimalPrecision);
    }
}
