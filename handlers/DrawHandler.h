#ifndef DRAWHANDLER_H
#define DRAWHANDLER_H

#include "IInteractionHandler.h"
#include <QPointF>
#include <QColor>
#include <QPen>
#include <QBrush>
#include <QGraphicsRectItem>
#include <QFont>

class QGraphicsScene;
class QGraphicsItem;
class QGraphicsRectItem;
class QGraphicsLineItem;
class QUndoStack;

/**
 * @brief 带标签的矩形图元类。
 *
 * 继承自 QGraphicsRectItem，在矩形内部绘制标签文本。
 * 主要用于石材切割场景中显示切割区域的名称。
 */
class LabeledRectItem : public QGraphicsRectItem
{
public:
    /**
     * @brief 构造函数。
     *
     * @param rect 矩形边界。
     * @param parent 父图元。
     */
    explicit LabeledRectItem(const QRectF &rect, QGraphicsItem *parent = nullptr);

    /**
     * @brief 构造函数。
     *
     * @param x 矩形左上角 x 坐标。
     * @param y 矩形左上角 y 坐标。
     * @param width 矩形宽度。
     * @param height 矩形高度。
     * @param parent 父图元。
     */
    LabeledRectItem(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent = nullptr);

    /**
     * @brief 设置标签文本。
     *
     * @param text 标签文本内容。
     */
    void setLabelText(const QString &text);

    /**
     * @brief 获取标签文本。
     *
     * @return 当前标签文本。
     */
    QString labelText() const;

    /**
     * @brief 设置标签字体。
     *
     * @param font 字体对象。
     */
    void setLabelFont(const QFont &font);

    /**
     * @brief 获取标签字体。
     *
     * @return 当前字体对象。
     */
    QFont labelFont() const;

    /**
     * @brief 设置标签颜色。
     *
     * @param color 文本颜色。
     */
    void setLabelColor(const QColor &color);

    /**
     * @brief 获取标签颜色。
     *
     * @return 当前文本颜色。
     */
    QColor labelColor() const;

    /**
     * @brief 重写绘制方法，在矩形内绘制标签。
     *
     * @param painter 绘制器。
     * @param option 样式选项。
     * @param widget 父控件。
     */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    /**
     * @brief 重写边界矩形计算，考虑标签文本。
     *
     * @return 图元边界矩形。
     */
    QRectF boundingRect() const override;

private:
    /**
     * @brief 标签文本内容。
     */
    QString m_labelText;

    /**
     * @brief 标签字体。
     */
    QFont m_labelFont;

    /**
     * @brief 标签文本颜色。
     */
    QColor m_labelColor;
};

/**
 * @brief 图形绘制处理器。
 *
 * 支持绘制矩形和线条，并可对绘制的图形进行命名。
 * 主要用于石材切割场景和楼层设备场景的图形标注。
 */
class DrawHandler : public AbstractInteractionHandler
{
    Q_OBJECT

public:
    /**
     * @brief 绘制模式枚举。
     */
    enum class DrawMode
    {
        None,       ///< 不绘制
        RectOnly,   ///< 仅绘制矩形
        LineOnly,   ///< 仅绘制线条
        RectAndLine ///< 绘制矩形和线条
    };

    /**
     * @brief 图形类型枚举。
     */
    enum class ShapeType
    {
        Rect,   ///< 矩形
        Line    ///< 线条
    };

    /**
     * @brief 构造函数。
     *
     * @param mode 绘制模式，默认为 RectAndLine。
     * @param enableNaming 是否启用命名功能，默认为 true。
     * @param namePrefix 图形名称前缀，默认为 "Shape"。
     * @param priority 处理器优先级，默认为 20。
     * @param parent 父 QObject 对象。
     */
    explicit DrawHandler(DrawMode mode = DrawMode::RectAndLine,
                         bool enableNaming = true,
                         const QString &namePrefix = "Shape",
                         int priority = 20,
                         QObject *parent = nullptr);

    /**
     * @brief 析构函数。
     */
    ~DrawHandler() override;

    /**
     * @brief 处理鼠标按下事件，开始绘制。
     */
    bool handleMousePress(QGraphicsView *view, QMouseEvent *event) override;

    /**
     * @brief 处理鼠标移动事件，更新绘制预览。
     */
    bool handleMouseMove(QGraphicsView *view, QMouseEvent *event) override;

    /**
     * @brief 处理鼠标释放事件，完成绘制。
     */
    bool handleMouseRelease(QGraphicsView *view, QMouseEvent *event) override;

    /**
     * @brief 处理键盘按下事件，支持快捷键切换模式。
     */
    bool handleKeyPress(QGraphicsView *view, QKeyEvent *event) override;

    /**
     * @brief 获取处理器名称标识。
     */
    QString handlerName() const override;

    // ========== 模式设置 ==========

    /**
     * @brief 设置绘制模式。
     *
     * @param mode 绘制模式。
     */
    void setDrawMode(DrawMode mode);

    /**
     * @brief 获取当前绘制模式。
     *
     * @return 当前绘制模式。
     */
    DrawMode drawMode() const;

    /**
     * @brief 设置当前绘制图形类型。
     *
     * 仅在 DrawMode 为 RectAndLine 时有效。
     *
     * @param type 图形类型。
     */
    void setCurrentShapeType(ShapeType type);

    /**
     * @brief 获取当前绘制图形类型。
     *
     * @return 当前图形类型。
     */
    ShapeType currentShapeType() const;

    /**
     * @brief 设置是否启用命名功能。
     *
     * @param enable true 启用；false 禁用。
     */
    void setEnableNaming(bool enable);

    /**
     * @brief 判断是否启用命名功能。
     *
     * @return true 启用；false 禁用。
     */
    bool isNamingEnabled() const;

    /**
     * @brief 设置图形名称前缀。
     *
     * @param prefix 名称前缀，图形将命名为 "前缀_编号"。
     */
    void setNamePrefix(const QString &prefix);

    /**
     * @brief 获取图形名称前缀。
     *
     * @return 当前名称前缀。
     */
    QString namePrefix() const;

    // ========== 绘制激活状态 ==========

    /**
     * @brief 设置是否激活绘制模式。
     *
     * 激活时，鼠标操作用于绘制图形；
     * 禁用时，鼠标操作用于选择和拖拽已有图形。
     *
     * @param active true 激活绘制模式；false 禁用（选择/拖拽模式）。
     */
    void setDrawingActive(bool active);

    /**
     * @brief 判断绘制模式是否激活。
     *
     * @return true 绘制模式激活；false 选择/拖拽模式。
     */
    bool isDrawingActive() const;

    /**
     * @brief 切换绘制模式状态。
     */
    void toggleDrawingMode();

    // ========== 样式设置 ==========

    /**
     * @brief 设置矩形画笔。
     *
     * @param pen 画笔对象。
     */
    void setRectPen(const QPen &pen);

    /**
     * @brief 获取矩形画笔。
     *
     * @return 当前画笔对象。
     */
    QPen rectPen() const;

    /**
     * @brief 设置矩形画刷。
     *
     * @param brush 画刷对象。
     */
    void setRectBrush(const QBrush &brush);

    /**
     * @brief 获取矩形画刷。
     *
     * @return 当前画刷对象。
     */
    QBrush rectBrush() const;

    /**
     * @brief 设置线条画笔。
     *
     * @param pen 画笔对象。
     */
    void setLinePen(const QPen &pen);

    /**
     * @brief 获取线条画笔。
     *
     * @return 当前画笔对象。
     */
    QPen linePen() const;

    // ========== 场景关联 ==========

    /**
     * @brief 设置关联的场景。
     *
     * @param scene QGraphicsScene 实例。
     */
    void setScene(QGraphicsScene *scene);

    /**
     * @brief 获取关联的场景。
     *
     * @return 关联的 QGraphicsScene 实例。
     */
    QGraphicsScene *scene() const;

    /**
     * @brief 设置撤销栈，用于支持撤销/重做。
     *
     * @param undoStack 撤销栈对象。
     */
    void setUndoStack(QUndoStack *undoStack);

    /**
     * @brief 取消当前正在进行的绘制操作。
     */
    void cancelDrawing();

    /**
     * @brief 判断是否正在绘制。
     *
     * @return true 正在绘制；false 空闲状态。
     */
    bool isDrawing() const;

signals:
    /**
     * @brief 图形绘制完成信号。
     *
     * @param item 创建的图元对象。
     * @param shapeType 图形类型。
     * @param name 图形名称。
     */
    void shapeCreated(QGraphicsItem *item, ShapeType shapeType, const QString &name);

    /**
     * @brief 绘制开始信号。
     */
    void drawingStarted();

    /**
     * @brief 绘制取消信号。
     */
    void drawingCancelled();

    /**
     * @brief 绘制激活状态改变信号。
     *
     * 当绘制/选择模式切换时发出。
     *
     * @param active true 绘制模式；false 选择模式。
     */
    void drawingActiveChanged(bool active);

private:
    /**
     * @brief 开始绘制矩形。
     *
     * @param scenePos 场景坐标起点。
     */
    void startDrawingRect(const QPointF &scenePos);

    /**
     * @brief 开始绘制线条。
     *
     * @param scenePos 场景坐标起点。
     */
    void startDrawingLine(const QPointF &scenePos);

    /**
     * @brief 更新矩形预览。
     *
     * @param scenePos 当前场景坐标。
     */
    void updateRect(const QPointF &scenePos);

    /**
     * @brief 更新线条预览。
     *
     * @param scenePos 当前场景坐标。
     */
    void updateLine(const QPointF &scenePos);

    /**
     * @brief 完成矩形绘制。
     */
    void finishRect();

    /**
     * @brief 完成线条绘制。
     */
    void finishLine();

    /**
     * @brief 生成下一个图形名称。
     *
     * @return 生成的图形名称。
     */
    QString generateShapeName() const;

    /**
     * @brief 移除预览图元。
     */
    void removePreviewItem();

private:
    /**
     * @brief 关联的场景。
     */
    QGraphicsScene *m_scene;

    /**
     * @brief 绘制模式。
     */
    DrawMode m_drawMode;

    /**
     * @brief 当前绘制图形类型。
     */
    ShapeType m_currentShapeType;

    /**
     * @brief 是否启用命名。
     */
    bool m_enableNaming;

    /**
     * @brief 名称前缀。
     */
    QString m_namePrefix;

    /**
     * @brief 图形计数器。
     */
    mutable int m_shapeCounter;

    /**
     * @brief 矩形画笔。
     */
    QPen m_rectPen;

    /**
     * @brief 矩形画刷。
     */
    QBrush m_rectBrush;

    /**
     * @brief 线条画笔。
     */
    QPen m_linePen;

    /**
     * @brief 绘制模式是否激活。
     *
     * true 绘制模式，鼠标操作用于绘制图形；
     * false 选择/拖拽模式，鼠标操作用于选择和移动已有图形。
     */
    bool m_drawingActive;

    /**
     * @brief 是否正在绘制。
     */
    bool m_isDrawing;

    /**
     * @brief 绘制起点（场景坐标）。
     */
    QPointF m_startPos;

    /**
     * @brief 矩形预览图元。
     */
    LabeledRectItem *m_rectPreview;

    /**
     * @brief 线条预览图元。
     */
    QGraphicsLineItem *m_linePreview;

    /**
     * @brief 撤销栈，用于支持撤销/重做。
     */
    QUndoStack *m_undoStack;
};

#endif // DRAWHANDLER_H
