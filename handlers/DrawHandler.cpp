#include "DrawHandler.h"

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsLineItem>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QInputDialog>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QFontMetrics>

// ==================== LabeledRectItem 实现 ====================

LabeledRectItem::LabeledRectItem(const QRectF &rect, QGraphicsItem *parent)
    : QGraphicsRectItem(rect, parent)
    , m_labelText()
    , m_labelFont("Arial", 10)
    , m_labelColor(Qt::white)
{
}

LabeledRectItem::LabeledRectItem(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent)
    : QGraphicsRectItem(x, y, width, height, parent)
    , m_labelText()
    , m_labelFont("Arial", 10)
    , m_labelColor(Qt::white)
{
}

void LabeledRectItem::setLabelText(const QString &text)
{
    m_labelText = text;
    update();
}

QString LabeledRectItem::labelText() const
{
    return m_labelText;
}

void LabeledRectItem::setLabelFont(const QFont &font)
{
    m_labelFont = font;
    update();
}

QFont LabeledRectItem::labelFont() const
{
    return m_labelFont;
}

void LabeledRectItem::setLabelColor(const QColor &color)
{
    m_labelColor = color;
    update();
}

QColor LabeledRectItem::labelColor() const
{
    return m_labelColor;
}

void LabeledRectItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // 先绘制矩形基类
    QGraphicsRectItem::paint(painter, option, widget);

    // 如果有标签文本，在矩形内部居中绘制
    if (!m_labelText.isEmpty()) {
        QRectF rect = this->rect();

        // 设置字体和颜色
        painter->setFont(m_labelFont);
        painter->setPen(m_labelColor);

        // 绘制背景（可选，提高可读性）
        QFontMetrics fm(m_labelFont);
        QRect textRect = fm.boundingRect(m_labelText);
        textRect.moveCenter(rect.center().toPoint());

        // 绘制半透明背景
        QColor bgColor(0, 0, 0, 120);
        painter->fillRect(textRect.adjusted(-4, -2, 4, 2), bgColor);

        // 绘制文本（居中）
        painter->drawText(rect, Qt::AlignCenter, m_labelText);
    }
}

QRectF LabeledRectItem::boundingRect() const
{
    return QGraphicsRectItem::boundingRect();
}

// ==================== DrawHandler 实现 ====================

DrawHandler::DrawHandler(DrawMode mode, bool enableNaming,
                         const QString &namePrefix, int priority, QObject *parent)
    : AbstractInteractionHandler(priority, parent)
    , m_scene(nullptr)
    , m_drawMode(mode)
    , m_currentShapeType(ShapeType::Rect)
    , m_enableNaming(enableNaming)
    , m_namePrefix(namePrefix)
    , m_shapeCounter(0)
    , m_rectPen(QColor(0, 100, 200), 2)
    , m_rectBrush(QColor(100, 149, 237, 80))
    , m_linePen(QColor(200, 50, 50), 2)
    , m_drawingActive(true)
    , m_isDrawing(false)
    , m_rectPreview(nullptr)
    , m_linePreview(nullptr)
{
}

DrawHandler::~DrawHandler()
{
    cancelDrawing();
}

QString DrawHandler::handlerName() const
{
    return QStringLiteral("DrawHandler");
}

bool DrawHandler::handleMousePress(QGraphicsView *view, QMouseEvent *event)
{
    // 绘制模式未激活时，不处理事件，允许选择/拖拽
    if (!m_drawingActive) {
        return false;
    }

    if (!m_scene || m_drawMode == DrawMode::None) {
        return false;
    }

    // 仅处理左键
    if (event->button() != Qt::LeftButton) {
        return false;
    }

    QPointF scenePos = view->mapToScene(event->pos());

    // 根据当前模式决定绘制类型
    ShapeType drawType = m_currentShapeType;
    if (m_drawMode == DrawMode::RectOnly) {
        drawType = ShapeType::Rect;
    } else if (m_drawMode == DrawMode::LineOnly) {
        drawType = ShapeType::Line;
    }

    // 检查是否支持该类型
    if (m_drawMode == DrawMode::RectOnly && drawType == ShapeType::Line) {
        return false;
    }
    if (m_drawMode == DrawMode::LineOnly && drawType == ShapeType::Rect) {
        return false;
    }

    m_isDrawing = true;
    m_startPos = scenePos;

    if (drawType == ShapeType::Rect) {
        startDrawingRect(scenePos);
    } else {
        startDrawingLine(scenePos);
    }

    emit drawingStarted();
    return true;
}

bool DrawHandler::handleMouseMove(QGraphicsView *view, QMouseEvent *event)
{
    if (!m_isDrawing) {
        return false;
    }

    QPointF scenePos = view->mapToScene(event->pos());

    if (m_rectPreview) {
        updateRect(scenePos);
    } else if (m_linePreview) {
        updateLine(scenePos);
    }

    return true;
}

bool DrawHandler::handleMouseRelease(QGraphicsView *view, QMouseEvent *event)
{
    Q_UNUSED(view)

    if (!m_isDrawing) {
        return false;
    }

    if (event->button() != Qt::LeftButton) {
        return false;
    }

    if (m_rectPreview) {
        finishRect();
    } else if (m_linePreview) {
        finishLine();
    }

    // 注意：finishRect/finishLine 已经设置了 m_isDrawing = false
    return true;
}

bool DrawHandler::handleKeyPress(QGraphicsView *view, QKeyEvent *event)
{
    Q_UNUSED(view)

    // Escape 键取消绘制
    if (event->key() == Qt::Key_Escape) {
        if (m_isDrawing) {
            cancelDrawing();
            return true;
        }
    }

    // R 键切换到矩形模式
    if (event->key() == Qt::Key_R && m_drawMode == DrawMode::RectAndLine) {
        m_currentShapeType = ShapeType::Rect;
        return true;
    }

    // L 键切换到线条模式
    if (event->key() == Qt::Key_L && m_drawMode == DrawMode::RectAndLine) {
        m_currentShapeType = ShapeType::Line;
        return true;
    }

    return false;
}

void DrawHandler::setDrawMode(DrawMode mode)
{
    m_drawMode = mode;

    // 自动调整当前类型
    if (mode == DrawMode::RectOnly) {
        m_currentShapeType = ShapeType::Rect;
    } else if (mode == DrawMode::LineOnly) {
        m_currentShapeType = ShapeType::Line;
    }
}

DrawHandler::DrawMode DrawHandler::drawMode() const
{
    return m_drawMode;
}

void DrawHandler::setCurrentShapeType(ShapeType type)
{
    if (m_drawMode == DrawMode::RectAndLine) {
        m_currentShapeType = type;
    }
}

DrawHandler::ShapeType DrawHandler::currentShapeType() const
{
    return m_currentShapeType;
}

void DrawHandler::setEnableNaming(bool enable)
{
    m_enableNaming = enable;
}

bool DrawHandler::isNamingEnabled() const
{
    return m_enableNaming;
}

void DrawHandler::setNamePrefix(const QString &prefix)
{
    m_namePrefix = prefix;
}

QString DrawHandler::namePrefix() const
{
    return m_namePrefix;
}

void DrawHandler::setDrawingActive(bool active)
{
    m_drawingActive = active;
}

bool DrawHandler::isDrawingActive() const
{
    return m_drawingActive;
}

void DrawHandler::toggleDrawingMode()
{
    m_drawingActive = !m_drawingActive;
}

void DrawHandler::setRectPen(const QPen &pen)
{
    m_rectPen = pen;
}

QPen DrawHandler::rectPen() const
{
    return m_rectPen;
}

void DrawHandler::setRectBrush(const QBrush &brush)
{
    m_rectBrush = brush;
}

QBrush DrawHandler::rectBrush() const
{
    return m_rectBrush;
}

void DrawHandler::setLinePen(const QPen &pen)
{
    m_linePen = pen;
}

QPen DrawHandler::linePen() const
{
    return m_linePen;
}

void DrawHandler::setScene(QGraphicsScene *scene)
{
    m_scene = scene;
}

QGraphicsScene *DrawHandler::scene() const
{
    return m_scene;
}

void DrawHandler::cancelDrawing()
{
    if (!m_isDrawing) {
        return;
    }

    removePreviewItem();
    m_isDrawing = false;
    emit drawingCancelled();
}

bool DrawHandler::isDrawing() const
{
    return m_isDrawing;
}

void DrawHandler::startDrawingRect(const QPointF &scenePos)
{
    if (!m_scene) {
        return;
    }

    // 使用 LabeledRectItem 代替普通矩形
    m_rectPreview = new LabeledRectItem(QRectF(scenePos, QSizeF(0, 0)));
    m_rectPreview->setPen(m_rectPen);
    m_rectPreview->setBrush(m_rectBrush);
    m_rectPreview->setZValue(100);
    m_scene->addItem(m_rectPreview);
}

void DrawHandler::startDrawingLine(const QPointF &scenePos)
{
    if (!m_scene) {
        return;
    }

    m_linePreview = m_scene->addLine(QLineF(scenePos, scenePos), m_linePen);
    m_linePreview->setZValue(100);
}

void DrawHandler::updateRect(const QPointF &scenePos)
{
    if (!m_rectPreview) {
        return;
    }

    QRectF rect = QRectF(m_startPos, scenePos).normalized();
    m_rectPreview->setRect(rect);
}

void DrawHandler::updateLine(const QPointF &scenePos)
{
    if (!m_linePreview) {
        return;
    }

    m_linePreview->setLine(QLineF(m_startPos, scenePos));
}

void DrawHandler::finishRect()
{
    if (!m_rectPreview) {
        return;
    }

    // 检查矩形是否有有效大小
    QRectF rect = m_rectPreview->rect();
    if (rect.width() < 5 || rect.height() < 5) {
        removePreviewItem();
        m_isDrawing = false;
        return;
    }

    // 生成名称
    QString name = generateShapeName();

    // 如果启用命名，弹出对话框让用户输入名称
    if (m_enableNaming) {
        bool ok;
        QString inputName = QInputDialog::getText(nullptr,
            tr("命名图形"),
            tr("请输入图形名称:"),
            QLineEdit::Normal,
            name,
            &ok);
        if (!ok) {
            // 用户点击取消，删除预览图元
            removePreviewItem();
            m_isDrawing = false;
            return;
        }
        if (!inputName.isEmpty()) {
            name = inputName;
        }
    }

    // 设置图形属性
    m_rectPreview->setFlag(QGraphicsItem::ItemIsSelectable);
    m_rectPreview->setFlag(QGraphicsItem::ItemIsMovable);
    m_rectPreview->setData(0, "DrawShape");
    m_rectPreview->setData(1, static_cast<int>(ShapeType::Rect));
    m_rectPreview->setData(2, name);

    // 设置矩形内的标签文本
    m_rectPreview->setLabelText(name);

    // 发送信号
    emit shapeCreated(m_rectPreview, ShapeType::Rect, name);

    m_rectPreview = nullptr;
    m_isDrawing = false;
    m_shapeCounter++;
}

void DrawHandler::finishLine()
{
    if (!m_linePreview) {
        return;
    }

    // 检查线条是否有有效长度
    QLineF line = m_linePreview->line();
    if (line.length() < 5) {
        removePreviewItem();
        m_isDrawing = false;
        return;
    }

    // 生成名称
    QString name = generateShapeName();

    // 如果启用命名，弹出对话框让用户输入名称
    if (m_enableNaming) {
        bool ok;
        QString inputName = QInputDialog::getText(nullptr,
            tr("命名图形"),
            tr("请输入线条名称:"),
            QLineEdit::Normal,
            name,
            &ok);
        if (!ok) {
            // 用户点击取消，删除预览图元
            removePreviewItem();
            m_isDrawing = false;
            return;
        }
        if (!inputName.isEmpty()) {
            name = inputName;
        }
    }

    // 设置图形属性
    m_linePreview->setFlag(QGraphicsItem::ItemIsSelectable);
    m_linePreview->setFlag(QGraphicsItem::ItemIsMovable);
    m_linePreview->setData(0, "DrawShape");
    m_linePreview->setData(1, static_cast<int>(ShapeType::Line));
    m_linePreview->setData(2, name);

    // 发送信号
    emit shapeCreated(m_linePreview, ShapeType::Line, name);

    m_linePreview = nullptr;
    m_isDrawing = false;
    m_shapeCounter++;
}

QString DrawHandler::generateShapeName() const
{
    return QString("%1_%2").arg(m_namePrefix).arg(m_shapeCounter + 1);
}

void DrawHandler::removePreviewItem()
{
    if (m_rectPreview && m_scene) {
        m_scene->removeItem(m_rectPreview);
        delete m_rectPreview;
        m_rectPreview = nullptr;
    }
    if (m_linePreview && m_scene) {
        m_scene->removeItem(m_linePreview);
        delete m_linePreview;
        m_linePreview = nullptr;
    }
}
