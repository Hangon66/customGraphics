#ifndef GUIDELINEHANDLER_H
#define GUIDELINEHANDLER_H

#include "IInteractionHandler.h"
#include <QList>
#include <QColor>

class QGraphicsView;
class QPainter;

/**
 * @brief 辅助线数据结构。
 *
 * 表示一条水平或垂直的辅助线，存储其方向和场景坐标位置。
 */
struct GuideLine
{
    /**
     * @brief 辅助线方向枚举。
     */
    enum Type
    {
        Horizontal, ///< 水平辅助线
        Vertical     ///< 垂直辅助线
    };

    /**
     * @brief 辅助线方向。
     */
    Type type;

    /**
     * @brief 辅助线在场景中的坐标位置。
     *
     * 水平辅助线为 Y 坐标，垂直辅助线为 X 坐标。
     */
    qreal position;
};

/**
 * @brief 辅助线交互处理器。
 *
 * 提供类似 Photoshop 的辅助线功能，支持从标尺区域拖拽创建辅助线、
 * 拖拽移动已有辅助线、拖拽到标尺区域删除辅助线。
 * 辅助线以青色虚线绘制在视图前景层，拖拽中的辅助线以品红色高亮显示。
 */
class GuideLineHandler : public AbstractInteractionHandler
{
    Q_OBJECT

public:
    /**
     * @brief 水平标尺位置枚举。
     *
     * 决定水平标尺在视口的顶部还是底部，
     * 影响辅助线的拖拽创建区域和三角形标记位置。
     */
    enum class RulerPosition
    {
        Top,    ///< 顶部水平标尺
        Bottom  ///< 底部水平标尺
    };

    /**
     * @brief 拖拽状态枚举。
     */
    enum class DragState
    {
        None,     ///< 无拖拽
        Creating, ///< 正在从标尺创建辅助线
        Moving    ///< 正在移动已有辅助线
    };

    /**
     * @brief 构造函数。
     *
     * @param priority 处理器优先级，默认 80（高于 Pan 50，低于 Zoom 100）。
     * @param parent 父 QObject 对象。
     */
    explicit GuideLineHandler(int priority = 80, QObject *parent = nullptr);

    /**
     * @brief 析构函数。
     */
    ~GuideLineHandler() override;

    /**
     * @brief 获取处理器名称标识。
     *
     * @return 固定返回 "GuideLineHandler"。
     */
    QString handlerName() const override;

    /**
     * @brief 处理鼠标按下事件。
     *
     * 检测标尺区域点击以创建辅助线，或检测辅助线点击以开始移动。
     *
     * @param view 产生事件的 QGraphicsView 实例。
     * @param event 鼠标按下事件对象。
     * @return true 表示事件已被消费；false 表示未处理。
     */
    bool handleMousePress(QGraphicsView *view, QMouseEvent *event) override;

    /**
     * @brief 处理鼠标移动事件。
     *
     * 更新正在拖拽的辅助线位置。
     *
     * @param view 产生事件的 QGraphicsView 实例。
     * @param event 鼠标移动事件对象。
     * @return true 表示事件已被消费；false 表示未处理。
     */
    bool handleMouseMove(QGraphicsView *view, QMouseEvent *event) override;

    /**
     * @brief 处理鼠标释放事件。
     *
     * 完成辅助线创建或移动，若拖拽到标尺区域则删除辅助线。
     *
     * @param view 产生事件的 QGraphicsView 实例。
     * @param event 鼠标释放事件对象。
     * @return true 表示事件已被消费；false 表示未处理。
     */
    bool handleMouseRelease(QGraphicsView *view, QMouseEvent *event) override;

    /**
     * @brief 绘制辅助线。
     *
     * 在视图前景层绘制所有辅助线，拖拽中的辅助线以高亮色显示。
     * 标尺区域绘制辅助线位置的小三角形标记。
     *
     * @param painter 绘制器。
     * @param view 关联的视图。
     */
    void paint(QPainter *painter, QGraphicsView *view);

    // ========== 辅助线管理接口 ==========

    /**
     * @brief 添加一条辅助线。
     *
     * @param type 辅助线方向。
     * @param position 场景坐标位置。
     */
    void addGuideLine(GuideLine::Type type, qreal position);

    /**
     * @brief 移除指定索引的辅助线。
     *
     * @param index 辅助线索引，需在有效范围内。
     */
    void removeGuideLine(int index);

    /**
     * @brief 清除所有辅助线。
     */
    void clearGuideLines();

    /**
     * @brief 获取辅助线数量。
     *
     * @return 当前辅助线的数量。
     */
    int guideLineCount() const;

    /**
     * @brief 获取指定索引的辅助线。
     *
     * @param index 辅助线索引。
     * @return 辅助线数据。
     */
    GuideLine guideLine(int index) const;

    /**
     * @brief 设置辅助线是否可见。
     *
     * @param visible true 显示辅助线；false 隐藏辅助线。
     */
    void setGuideLineVisible(bool visible);

    /**
     * @brief 获取辅助线是否可见。
     *
     * @return true 可见；false 隐藏。
     */
    bool isGuideLineVisible() const;

    /**
     * @brief 设置标尺宽度。
     *
     * 需要与 RulerHandler 的标尺宽度保持一致，
     * 用于判断鼠标是否在标尺区域内。
     *
     * @param width 标尺宽度（像素）。
     */
    void setRulerWidth(int width);

    /**
     * @brief 获取标尺宽度。
     *
     * @return 标尺宽度（像素）。
     */
    int rulerWidth() const;

    /**
     * @brief 设置水平标尺位置。
     *
     * 需要与 RulerHandler 的标尺位置保持一致，
     * 用于判断鼠标是否在水平标尺区域内以及三角形标记绘制位置。
     *
     * @param position 标尺位置（顶部或底部）。
     */
    void setRulerPosition(RulerPosition position);

    /**
     * @brief 获取水平标尺位置。
     *
     * @return 当前标尺位置。
     */
    RulerPosition rulerPosition() const;

    /**
     * @brief 设置辅助线颜色。
     *
     * @param color 辅助线颜色。
     */
    void setGuideLineColor(const QColor &color);

    /**
     * @brief 获取辅助线颜色。
     *
     * @return 当前辅助线颜色。
     */
    QColor guideLineColor() const;

    /**
     * @brief 设置拖拽中辅助线的高亮颜色。
     *
     * @param color 高亮颜色。
     */
    void setDragColor(const QColor &color);

    /**
     * @brief 获取拖拽中辅助线的高亮颜色。
     *
     * @return 当前高亮颜色。
     */
    QColor dragColor() const;

    /**
     * @brief 设置辅助线吸附阈值。
     *
     * 当鼠标距离辅助线小于此阈值时，认为鼠标在辅助线上。
     *
     * @param threshold 吸附阈值（视图像素），必须大于 0。
     */
    void setSnapThreshold(qreal threshold);

    /**
     * @brief 获取辅助线吸附阈值。
     *
     * @return 当前吸附阈值（视图像素）。
     */
    qreal snapThreshold() const;

    /**
     * @brief 设置指定辅助线的位置。
     *
     * 供外部（如属性面板编辑）修改辅助线位置时调用。
     *
     * @param index 辅助线索引，需在有效范围内。
     * @param position 新的场景坐标位置。
     */
    void setGuideLinePosition(int index, qreal position);

signals:
    /**
     * @brief 辅助线列表变化信号。
     *
     * 当辅助线被添加、移除或移动时发出。
     */
    void guideLinesChanged();

    /**
     * @brief 辅助线被点击选中信号。
     *
     * 当用户点击辅助线时发出，用于在属性面板中显示辅助线属性。
     *
     * @param index 被选中的辅助线索引。
     * @param type 辅助线方向（水平或垂直）。
     * @param position 辅助线的场景坐标位置。
     */
    void guideLineSelected(int index, GuideLine::Type type, qreal position);

    /**
     * @brief 辅助线拖动中位置更新信号。
     *
     * 拖动辅助线时持续发出，用于实时更新属性面板中的位置值。
     *
     * @param index 正在拖动的辅助线索引。
     * @param position 辅助线当前场景坐标位置。
     */
    void guideLineMoved(int index, qreal position);

    /**
     * @brief 辅助线取消选中信号。
     *
     * 拖动辅助线释放时发出，用于清除属性面板中的辅助线属性显示。
     */
    void guideLineDeselected();

private:
    /**
     * @brief 检测视图坐标是否在水平标尺区域内。
     *
     * 根据 m_rulerPosition 判断顶部或底部标尺区域。
     *
     * @param viewPos 视图坐标。
     * @param view 关联的视图。
     * @return true 在水平标尺区域内；false 不在。
     */
    bool isInHorizontalRuler(const QPoint &viewPos, QGraphicsView *view) const;

    /**
     * @brief 检测视图坐标是否在左侧标尺区域内。
     *
     * @param viewPos 视图坐标。
     * @param view 关联的视图。
     * @return true 在左侧标尺区域内；false 不在。
     */
    bool isInLeftRuler(const QPoint &viewPos, QGraphicsView *view) const;

    /**
     * @brief 查找鼠标位置附近的辅助线。
     *
     * @param viewPos 视图坐标。
     * @param view 关联的视图。
     * @return 匹配的辅助线索引，未找到返回 -1。
     */
    int findGuideLineAt(const QPoint &viewPos, QGraphicsView *view) const;

    /**
     * @brief 绘制标尺上的辅助线三角形标记。
     *
     * @param painter 绘制器。
     * @param view 关联的视图。
     */
    void paintRulerIndicators(QPainter *painter, QGraphicsView *view);

private:
    /**
     * @brief 辅助线列表。
     */
    QList<GuideLine> m_guideLines;

    /**
     * @brief 辅助线是否可见。
     */
    bool m_guideLineVisible;

    /**
     * @brief 当前拖拽状态。
     */
    DragState m_dragState;

    /**
     * @brief 正在拖拽的辅助线数据。
     */
    GuideLine m_dragGuideLine;

    /**
     * @brief 正在移动的辅助线索引（-1 表示无效）。
     */
    int m_dragIndex;

    /**
     * @brief 标尺宽度（像素），与 RulerHandler 保持一致。
     */
    int m_rulerWidth;

    /**
     * @brief 辅助线常规颜色。
     */
    QColor m_guideLineColor;

    /**
     * @brief 拖拽中辅助线的高亮颜色。
     */
    QColor m_dragColor;

    /**
     * @brief 辅助线吸附阈值（视图像素）。
     */
    qreal m_snapThreshold;

    /**
     * @brief 水平标尺位置（顶部或底部），与 RulerHandler 保持一致。
     */
    RulerPosition m_rulerPosition;
};

#endif // GUIDELINEHANDLER_H
