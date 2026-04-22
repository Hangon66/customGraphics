#include "DrawHandler.h"
#include "CollisionHandler.h"
#include "../commands/ShapeCommands.h"
#include "../view/CustomGraphicsScene.h"
#include "../view/ShapeMetadata.h"

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsLineItem>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QInputDialog>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionGraphicsItem>
#include <QFontMetrics>
#include <QUndoStack>
#include <cmath>

// ==================== LabeledRectItem 实现 ====================

// 将 DrawHandler::ShapeType 转换为 ShapeMeta::Type 的辅助函数
static ShapeMeta::Type toShapeMetaType(DrawHandler::ShapeType type)
{
    switch (type) {
    case DrawHandler::ShapeType::Rect:
        return ShapeMeta::Rect;
    case DrawHandler::ShapeType::Line:
        return ShapeMeta::Line;
    default:
        return ShapeMeta::Unknown;
    }
}

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

    // 如果有标签文本，在矩形中心绘制
    if (!m_labelText.isEmpty()) {
        QRectF rect = this->rect();

        // 设置字体和颜色
        painter->setFont(m_labelFont);
        painter->setPen(m_labelColor);

        // 计算文本尺寸
        QFontMetrics fm(m_labelFont);
        QRect textRect = fm.boundingRect(m_labelText);

        // 背景框位置：以矩形中心为中心
        int bgWidth = textRect.width() + 8;
        int bgHeight = textRect.height() + 4;
        qreal bgCenterX = rect.center().x();
        qreal bgCenterY = rect.center().y();

        // 绘制半透明背景
        QColor bgColor(0, 0, 0, 120);
        QRect bgRect(static_cast<int>(bgCenterX - bgWidth / 2.0),
                     static_cast<int>(bgCenterY - bgHeight / 2.0),
                     bgWidth, bgHeight);
        painter->fillRect(bgRect, bgColor);

        // 在背景框内居中绘制文本
        painter->drawText(bgRect, Qt::AlignCenter, m_labelText);
    }
}

QRectF LabeledRectItem::boundingRect() const
{
    QRectF baseRect = QGraphicsRectItem::boundingRect();

    if (!m_labelText.isEmpty()) {
        QFontMetrics fm(m_labelFont);
        QRect textRect = fm.boundingRect(m_labelText);

        // 计算背景框尺寸
        int bgWidth = textRect.width() + 8;
        int bgHeight = textRect.height() + 4;

        // 计算背景框可能超出矩形的部分
        qreal rectWidth = baseRect.width();
        qreal rectHeight = baseRect.height();

        // 如果背景框大于矩形，需要扩展边界
        qreal extraWidth = qMax(0.0, (bgWidth - rectWidth) / 2.0 + 2);
        qreal extraHeight = qMax(0.0, (bgHeight - rectHeight) / 2.0 + 2);

        if (extraWidth > 0 || extraHeight > 0) {
            baseRect.adjust(-extraWidth, -extraHeight, extraWidth, extraHeight);
        }
    }

    return baseRect;
}

QPainterPath LabeledRectItem::shape() const
{
    // 碰撞检测使用原始矩形，不包含标签区域
    QPainterPath path;
    path.addRect(rect());
    return path;
}

// ==================== LabeledLineItem 实现 ====================

LabeledLineItem::LabeledLineItem(const QLineF &line, QGraphicsItem *parent)
    : QGraphicsLineItem(line, parent)
    , m_labelText()
    , m_labelFont("Arial", 10)
    , m_labelColor(Qt::white)
{
}

LabeledLineItem::LabeledLineItem(qreal x1, qreal y1, qreal x2, qreal y2, QGraphicsItem *parent)
    : QGraphicsLineItem(x1, y1, x2, y2, parent)
    , m_labelText()
    , m_labelFont("Arial", 10)
    , m_labelColor(Qt::white)
{
}

void LabeledLineItem::setLabelText(const QString &text)
{
    m_labelText = text;
    update();
}

QString LabeledLineItem::labelText() const
{
    return m_labelText;
}

void LabeledLineItem::setLabelFont(const QFont &font)
{
    m_labelFont = font;
    update();
}

QFont LabeledLineItem::labelFont() const
{
    return m_labelFont;
}

void LabeledLineItem::setLabelColor(const QColor &color)
{
    m_labelColor = color;
    update();
}

QColor LabeledLineItem::labelColor() const
{
    return m_labelColor;
}

void LabeledLineItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // 先绘制线条基类
    QGraphicsLineItem::paint(painter, option, widget);

    // 如果有标签文本，在线条中点上方绘制
    if (!m_labelText.isEmpty()) {
        QLineF line = this->line();
        QPointF midPoint = line.center();

        // 设置字体和颜色
        painter->setFont(m_labelFont);
        painter->setPen(m_labelColor);

        // 计算文本尺寸
        QFontMetrics fm(m_labelFont);
        QRect textRect = fm.boundingRect(m_labelText);

        // 背景框尺寸
        int bgWidth = textRect.width() + 8;
        int bgHeight = textRect.height() + 4;

        // 背景框中心位置（在线条中点上方）
        qreal bgCenterX = midPoint.x();
        qreal bgCenterY = midPoint.y() - bgHeight / 2.0 - 8;  // 在线条上方留8像素间距

        // 绘制半透明背景
        QColor bgColor(0, 0, 0, 120);
        QRect bgRect(static_cast<int>(bgCenterX - bgWidth / 2.0),
                     static_cast<int>(bgCenterY - bgHeight / 2.0),
                     bgWidth, bgHeight);
        painter->fillRect(bgRect, bgColor);

        // 在背景框内居中绘制文本
        painter->drawText(bgRect, Qt::AlignCenter, m_labelText);
    }
}

QRectF LabeledLineItem::boundingRect() const
{
    QRectF baseRect = QGraphicsLineItem::boundingRect();

    if (!m_labelText.isEmpty()) {
        QFontMetrics fm(m_labelFont);
        QRect textRect = fm.boundingRect(m_labelText);

        // 扩展边界以包含标签（在线条上方，与 paint 中的计算一致）
        int bgHeight = textRect.height() + 4;
        qreal extraHeight = bgHeight + 8 + 4;  // 背景高度 + 间距 + 边距
        qreal extraWidth = (textRect.width() + 8) / 2.0 + 4;

        baseRect.adjust(-extraWidth, -extraHeight, extraWidth, 0);
    }

    return baseRect;
}

QPainterPath LabeledLineItem::shape() const
{
    // 碰撞检测使用原始线条，不包含标签区域
    QPainterPath path;
    path.moveTo(line().p1());
    path.lineTo(line().p2());
    return path;
}

// ==================== DrawHandler 辅助函数 ====================

int DrawHandler::shapeTypeShortcut(ShapeType type)
{
    switch (type) {
    case ShapeType::Rect: return Qt::Key_1;
    case ShapeType::Line: return Qt::Key_2;
    default: return Qt::Key_1;
    }
}

DrawHandler::ShapeType DrawHandler::shapeTypeFromShortcut(int key)
{
    switch (key) {
    case Qt::Key_1: return ShapeType::Rect;
    case Qt::Key_2: return ShapeType::Line;
    default: return ShapeType::Rect;
    }
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
    , m_undoStack(nullptr)
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

    // 使用当前图形类型
    ShapeType drawType = m_currentShapeType;

    // 碰撞检测：检查起点是否在与当前绘制类型有碰撞配置的图元内部
    if (m_scene) {
        // 获取碰撞配置（如果场景支持）
        const CollisionConfig *config = nullptr;
        CollisionConfig defaultConfig;
        CustomGraphicsScene *customScene = qobject_cast<CustomGraphicsScene*>(m_scene);
        if (customScene) {
            config = &customScene->collisionConfig();
        } else {
            defaultConfig.enableAllCollisions();
            config = &defaultConfig;
        }

        // 将绘制类型转换为碰撞类型
        ShapeMeta::Type sourceCollisionType = toShapeMetaType(drawType);

        QGraphicsItem *itemAtStart = CollisionHandler::pointInAnyItemWithConfig(
            m_scene, scenePos, nullptr, sourceCollisionType, *config);
        if (itemAtStart) {
            // 起点在碰撞图元内，选中该图元并允许拖动
            // 清除其他选中状态，只选中点击的图元
            m_scene->clearSelection();
            itemAtStart->setSelected(true);
            // 返回 false 让 QGraphicsView 处理拖动
            return false;
        }
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

    // D 键切换绘制/选择模式
    if (event->key() == Qt::Key_D) {
        toggleDrawingMode();
        return true;
    }

    // Escape 键取消绘制
    if (event->key() == Qt::Key_Escape) {
        if (m_isDrawing) {
            cancelDrawing();
            return true;
        }
    }

    // 数字键切换图形类型（在激活模式下且未正在绘制）
    if (m_drawMode == DrawMode::Active && !m_isDrawing) {
        int key = event->key();
        // 检查是否是有效的数字键（1-4）
        if (key >= Qt::Key_1 && key <= Qt::Key_1 + shapeTypeCount() - 1) {
            ShapeType newType = shapeTypeFromShortcut(key);
            if (newType != m_currentShapeType) {
                m_currentShapeType = newType;
                emit shapeTypeChanged(newType);
            }
            return true;
        }
    }

    return false;
}

void DrawHandler::setDrawMode(DrawMode mode)
{
    m_drawMode = mode;
}

DrawHandler::DrawMode DrawHandler::drawMode() const
{
    return m_drawMode;
}

void DrawHandler::setCurrentShapeType(ShapeType type)
{
    if (m_currentShapeType != type) {
        m_currentShapeType = type;
        emit shapeTypeChanged(type);
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
    emit drawingActiveChanged(m_drawingActive);
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

void DrawHandler::setUndoStack(QUndoStack *undoStack)
{
    m_undoStack = undoStack;
}

void DrawHandler::setBoundaryConstraint(const QRectF &boundary)
{
    m_boundaryConstraint = boundary;
}

void DrawHandler::clearBoundaryConstraint()
{
    m_boundaryConstraint = QRectF();
}

QRectF DrawHandler::boundaryConstraint() const
{
    return m_boundaryConstraint;
}

bool DrawHandler::hasBoundaryConstraint() const
{
    return !m_boundaryConstraint.isNull();
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

    // 使用 LabeledLineItem 代替普通线条
    m_linePreview = new LabeledLineItem(QLineF(scenePos, scenePos));
    m_linePreview->setPen(m_linePen);
    m_linePreview->setZValue(100);
    m_scene->addItem(m_linePreview);
}

void DrawHandler::updateRect(const QPointF &scenePos)
{
    if (!m_rectPreview) {
        return;
    }

    // 计算受限的终点位置
    QPointF constrainedPos = constrainRectEndpoint(scenePos);

    QRectF rect = QRectF(m_startPos, constrainedPos).normalized();
    m_rectPreview->setRect(rect);
}

void DrawHandler::updateLine(const QPointF &scenePos)
{
    if (!m_linePreview) {
        return;
    }

    // 计算受限的终点位置
    QPointF constrainedPos = constrainLineEndpoint(scenePos);

    m_linePreview->setLine(QLineF(m_startPos, constrainedPos));
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

    // 注意：终点位置已经在绘制过程中被限制，无需再次检查碰撞

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
    m_rectPreview->setData(ShapeMeta::Category, "DrawShape");
    m_rectPreview->setData(ShapeMeta::ShapeType, ShapeMeta::Rect);
    m_rectPreview->setData(ShapeMeta::Name, name);
    PropMap rectProps;
    rectProps["typeName"] = PropField("矩形", "类型:", PropType::Text, true, false);
    rectProps["x"] = PropField(0.0, "X:", PropType::Number, true, true);
    rectProps["y"] = PropField(0.0, "Y:", PropType::Number, true, true);
    rectProps["width"] = PropField(0.0, "宽度:", PropType::Number, true, true);
    rectProps["height"] = PropField(0.0, "高度:", PropType::Number, true, true);
    m_rectPreview->setData(ShapeMeta::Props, QVariant::fromValue(rectProps));

    // 设置矩形内的标签文本
    m_rectPreview->setLabelText(name);

    // 先从场景移除预览图元（因为命令会重新添加）
    if (m_scene) {
        m_scene->removeItem(m_rectPreview);
    }

    // 使用命令模式创建图形，支持撤销
    if (m_undoStack) {
        CreateShapeCommand *cmd = new CreateShapeCommand(m_scene, m_rectPreview);
        cmd->setText(tr("创建矩形 %1").arg(name));
        m_undoStack->push(cmd);
    } else if (m_scene) {
        // 无撤销栈时直接添加
        m_scene->addItem(m_rectPreview);
    }

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

    // 注意：终点位置已经在绘制过程中被限制，无需再次检查碰撞

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
    m_linePreview->setData(ShapeMeta::Category, "DrawShape");
    m_linePreview->setData(ShapeMeta::ShapeType, ShapeMeta::Line);
    m_linePreview->setData(ShapeMeta::Name, name);
    PropMap lineProps;
    lineProps["typeName"] = PropField("线条", "类型:", PropType::Text, true, false);
    lineProps["x"] = PropField(0.0, "X:", PropType::Number, true, true);
    lineProps["y"] = PropField(0.0, "Y:", PropType::Number, true, true);
    lineProps["length"] = PropField(0.0, "长度:", PropType::Number, true, true);
    m_linePreview->setData(ShapeMeta::Props, QVariant::fromValue(lineProps));

    // 设置线条标签文本
    m_linePreview->setLabelText(name);

    // 先从场景移除预览图元（因为命令会重新添加）
    if (m_scene) {
        m_scene->removeItem(m_linePreview);
    }

    // 使用命令模式创建图形，支持撤销
    if (m_undoStack) {
        CreateShapeCommand *cmd = new CreateShapeCommand(m_scene, m_linePreview);
        cmd->setText(tr("创建线条 %1").arg(name));
        m_undoStack->push(cmd);
    } else if (m_scene) {
        // 无撤销栈时直接添加
        m_scene->addItem(m_linePreview);
    }

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

QPointF DrawHandler::constrainRectEndpoint(const QPointF &targetPos)
{
    QPointF constrainedPos = targetPos;

    // 第一步：应用边界约束
    if (hasBoundaryConstraint()) {
        QRectF targetRect = QRectF(m_startPos, targetPos).normalized();

        // 检查起点是否在边界内
        if (!m_boundaryConstraint.contains(m_startPos)) {
            // 起点不在边界内，不允许绘制
            return m_startPos;
        }

        // 将目标矩形约束到边界内
        QRectF constrainedRect = targetRect;

        // 约束右边界
        if (constrainedRect.right() > m_boundaryConstraint.right()) {
            constrainedRect.setRight(m_boundaryConstraint.right());
        }
        // 约束左边界
        if (constrainedRect.left() < m_boundaryConstraint.left()) {
            constrainedRect.setLeft(m_boundaryConstraint.left());
        }
        // 约束下边界
        if (constrainedRect.bottom() > m_boundaryConstraint.bottom()) {
            constrainedRect.setBottom(m_boundaryConstraint.bottom());
        }
        // 约束上边界
        if (constrainedRect.top() < m_boundaryConstraint.top()) {
            constrainedRect.setTop(m_boundaryConstraint.top());
        }

        // 计算约束后的终点位置
        // 根据原始拖动方向确定约束后的终点
        if (targetPos.x() >= m_startPos.x()) {
            constrainedPos.setX(constrainedRect.right());
        } else {
            constrainedPos.setX(constrainedRect.left());
        }
        if (targetPos.y() >= m_startPos.y()) {
            constrainedPos.setY(constrainedRect.bottom());
        } else {
            constrainedPos.setY(constrainedRect.top());
        }
    }

    // 第二步：应用碰撞约束
    if (!m_scene) {
        return constrainedPos;
    }

    // 获取碰撞配置
    const CollisionConfig *config = nullptr;
    CollisionConfig defaultConfig;
    CustomGraphicsScene *customScene = qobject_cast<CustomGraphicsScene*>(m_scene);
    if (customScene) {
        config = &customScene->collisionConfig();
    } else {
        defaultConfig.enableAllCollisions();
        config = &defaultConfig;
    }

    // 检查约束后的位置是否有效
    QRectF targetRect = QRectF(m_startPos, constrainedPos).normalized();
    if (!CollisionHandler::rectOverlapsAnyItem(m_scene, targetRect, nullptr, config)) {
        return constrainedPos;  // 无碰撞，直接返回
    }

    // 使用二分查找找到最大有效位置
    double tMin = 0.0;
    double tMax = 1.0;
    const int maxIterations = 20;
    const double tolerance = 0.5;

    QPointF direction = constrainedPos - m_startPos;
    double length = std::sqrt(direction.x() * direction.x() + direction.y() * direction.y());

    if (length < tolerance) {
        return m_startPos;
    }

    for (int i = 0; i < maxIterations; ++i) {
        double tMid = (tMin + tMax) / 2.0;
        QPointF midPos = m_startPos + direction * tMid;
        QRectF midRect = QRectF(m_startPos, midPos).normalized();

        if (CollisionHandler::rectOverlapsAnyItem(m_scene, midRect, nullptr, config)) {
            tMax = tMid;
        } else {
            tMin = tMid;
        }

        // 检查精度是否足够
        if ((tMax - tMin) * length < tolerance) {
            break;
        }
    }

    // 返回最大有效位置
    // 由于 rectOverlapsAnyItem 已区分边界接触和真正重叠，无需收缩
    return m_startPos + direction * tMin;
}

QPointF DrawHandler::constrainLineEndpoint(const QPointF &targetPos)
{
    QPointF constrainedPos = targetPos;

    // 第一步：应用边界约束
    if (hasBoundaryConstraint()) {
        // 检查起点是否在边界内
        if (!m_boundaryConstraint.contains(m_startPos)) {
            return m_startPos;
        }

        // 将终点约束到边界内
        constrainedPos.setX(qBound(m_boundaryConstraint.left(), constrainedPos.x(), m_boundaryConstraint.right()));
        constrainedPos.setY(qBound(m_boundaryConstraint.top(), constrainedPos.y(), m_boundaryConstraint.bottom()));
    }

    // 第二步：应用碰撞约束
    if (!m_scene) {
        return constrainedPos;
    }

    // 获取碰撞配置
    const CollisionConfig *config = nullptr;
    CollisionConfig defaultConfig;
    CustomGraphicsScene *customScene = qobject_cast<CustomGraphicsScene*>(m_scene);
    if (customScene) {
        config = &customScene->collisionConfig();
    } else {
        defaultConfig.enableAllCollisions();
        config = &defaultConfig;
    }

    // 对于线条，检查终点是否在图元内部
    if (!CollisionHandler::pointInAnyItem(m_scene, constrainedPos, nullptr, config)) {
        return constrainedPos;  // 终点无碰撞，直接返回
    }

    // 使用二分查找找到最大有效位置
    double tMin = 0.0;
    double tMax = 1.0;
    const int maxIterations = 20;
    const double tolerance = 0.5;

    QPointF direction = constrainedPos - m_startPos;
    double length = std::sqrt(direction.x() * direction.x() + direction.y() * direction.y());

    if (length < tolerance) {
        return m_startPos;
    }

    for (int i = 0; i < maxIterations; ++i) {
        double tMid = (tMin + tMax) / 2.0;
        QPointF midPos = m_startPos + direction * tMid;

        if (CollisionHandler::pointInAnyItem(m_scene, midPos, nullptr, config)) {
            tMax = tMid;
        } else {
            tMin = tMid;
        }

        if ((tMax - tMin) * length < tolerance) {
            break;
        }
    }

    // 返回最大有效位置
    // 点在边界上不属于碰撞，无需收缩
    return m_startPos + direction * tMin;
}

