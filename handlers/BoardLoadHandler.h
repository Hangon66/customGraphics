#ifndef BOARDLOADHANDLER_H
#define BOARDLOADHANDLER_H

#include "IInteractionHandler.h"
#include "../../structure.h"
#include <QPixmap>
#include <QPolygonF>
#include <QPen>
#include <QBrush>
#include <QVector>
#include <QHash>
#include <QGraphicsRectItem>

class CustomGraphicsScene;
class QGraphicsItem;
class QGraphicsTextItem;
class QUndoStack;

/**
 * @brief 大板数据加载处理器。
 *
 * 负责加载大板JSON数据、背景图片和成品轮廓。
 * 集成到CustomGraphics框架，支持撤销/重做、边界约束等功能。
 */
class BoardLoadHandler : public AbstractInteractionHandler
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数。
     *
     * @param priority 处理器优先级，默认值0（不参与事件处理）。
     * @param parent 父QObject对象。
     */
    explicit BoardLoadHandler(int priority = 0, QObject *parent = nullptr);

    /**
     * @brief 析构函数。
     */
    ~BoardLoadHandler() override;

    /**
     * @brief 获取处理器名称标识。
     *
     * @return 固定返回"BoardLoadHandler"。
     */
    QString handlerName() const override;

    /**
     * @brief 加载大板数据。
     *
     * 从JSON文件加载大板数据，包括基础信息、背景图片和成品列表。
     *
     * @param filePath JSON文件路径。
     * @return true 加载成功；false 加载失败。
     */
    bool loadBoardData(const QString &filePath);

    /**
     * @brief 设置大板数据。
     *
     * 直接使用已解析的BoardData数据更新场景。
     *
     * @param data 大板数据。
     */
    void setBoardData(const BoardData &data);

    /**
     * @brief 获取当前大板数据。
     *
     * @return 当前大板数据的引用。
     */
    const BoardData& boardData() const { return m_boardData; }

    /**
     * @brief 处理鼠标按下事件，检测成品整体拖动。
     *
     * 当点击成品矩形或标签时，进入整体拖动模式，
     * 记录所有成品和标签的初始位置。
     *
     * @param view 产生事件的 QGraphicsView 实例。
     * @param event 鼠标按下事件对象。
     * @return true 表示事件已被消费；false 表示未处理。
     */
    bool handleMousePress(QGraphicsView *view, QMouseEvent *event) override;

    /**
     * @brief 处理鼠标移动事件，执行成品整体拖动。
     *
     * 在整体拖动模式下，计算偏移量并同步移动所有成品和标签。
     *
     * @param view 产生事件的 QGraphicsView 实例。
     * @param event 鼠标移动事件对象。
     * @return true 表示事件已被消费；false 表示未处理。
     */
    bool handleMouseMove(QGraphicsView *view, QMouseEvent *event) override;

    /**
     * @brief 处理鼠标释放事件，结束成品整体拖动。
     *
     * @param view 产生事件的 QGraphicsView 实例。
     * @param event 鼠标释放事件对象。
     * @return true 表示事件已被消费；false 表示未处理。
     */
    bool handleMouseRelease(QGraphicsView *view, QMouseEvent *event) override;

    /**
     * @brief 清除所有大板数据。
     *
     * 移除场景中的背景图片和所有成品图元。
     */
    void clearBoard();

    /**
     * @brief 设置关联的场景。
     *
     * @param scene QGraphicsScene实例。
     */
    void setScene(CustomGraphicsScene *scene);

    /**
     * @brief 设置关联的视图，用于加载后自动适配显示。
     *
     * @param view QGraphicsView实例。
     */
    void setView(QGraphicsView *view);

    /**
     * @brief 设置是否允许成品拖拽。
     *
     * 默认关闭，开启后用户可拖拽成品图元整体移动。
     *
     * @param enabled true 允许拖拽；false 禁止拖拽。
     */
    void setArtifactDragEnabled(bool enabled);

    /**
     * @brief 获取是否允许成品拖拽。
     *
     * @return true 允许拖拽；false 禁止拖拽。
     */
    bool isArtifactDragEnabled() const;

    /**
     * @brief 获取关联的场景。
     *
     * @return 关联的QGraphicsScene实例；未设置则返回nullptr。
     */
    CustomGraphicsScene *scene() const { return m_scene; }

    /**
     * @brief 设置撤销栈。
     *
     * @param undoStack 撤销栈对象。
     */
    void setUndoStack(QUndoStack *undoStack);

    /**
     * @brief 获取最后错误信息。
     *
     * @return 错误描述字符串。
     */
    QString lastError() const { return m_lastError; }

    /**
     * @brief 设置成品轮廓画笔。
     *
     * @param pen 画笔对象。
     */
    void setArtifactPen(const QPen &pen);

    /**
     * @brief 获取成品轮廓画笔。
     *
     * @return 当前画笔对象。
     */
    QPen artifactPen() const { return m_artifactPen; }

    /**
     * @brief 设置成品填充画刷。
     *
     * @param brush 画刷对象。
     */
    void setArtifactBrush(const QBrush &brush);

    /**
     * @brief 获取成品填充画刷。
     *
     * @return 当前画刷对象。
     */
    QBrush artifactBrush() const { return m_artifactBrush; }

    /**
     * @brief 设置是否显示成品标签。
     *
     * @param show true 显示；false 隐藏。
     */
    void setShowLabels(bool show);

    /**
     * @brief 获取是否显示成品标签。
     *
     * @return true 显示；false 隐藏。
     */
    bool showLabels() const { return m_showLabels; }

    /**
     * @brief 设置视图适配时的边距（用于避开标尺等区域）。
     *
     * @param leftMargin 左侧边距（像素）。
     * @param bottomMargin 底侧边距（像素）。
     */
    void setFitMargins(int leftMargin, int bottomMargin);

    /**
     * @brief 适应视图到背景图片。
     *
     * @param view 要适应的QGraphicsView。
     */
    void fitViewToBoard(QGraphicsView *view);

    /**
     * @brief 获取背景图片尺寸。
     *
     * @return 背景图片的尺寸；无背景时返回QSizeF()。
     */
    QSizeF boardSize() const;

    /**
     * @brief 移动所有成品和标签图元。
     *
     * 如果已创建整体旋转图元组，则对组整体移动；否则逐个移动。
     *
     * @param dx 水平偏移量（正值向右，负值向左）。
     * @param dy 垂直偏移量（正值向下，负值向上）。
     */
    void moveArtifacts(qreal dx, qreal dy);

    /**
     * @brief 将所有成品作为整体旋转。
     *
     * 首次旋转时创建 QGraphicsItemGroup，后续在同一个 group 上累积旋转角度。
     * 旋转中心为所有成品矩形的几何中心。
     *
     * @param angleDelta 旋转角度增量（角度制，正值顺时针，负值逆时针）。
     */
    void rotateArtifacts(qreal angleDelta);

    /**
     * @brief 设置所有成品的透明度。
     *
     * @param opacity 透明度值，范围 0.0（全透明）~1.0（不透明）。
     */
    void setArtifactsOpacity(qreal opacity);

    /**
     * @brief 调整所有成品填充色的亮度。
     *
     * 通过修改 HSV 颜色模型中的 Value 分量来调整亮度。
     *
     * @param delta 亮度增量（正值增亮，负值减暗）。
     */
    void adjustArtifactsBrightness(int delta);

    /**
     * @brief 设置成品偏移到指定绝对位置（场景坐标）。
     *
     * 计算与当前偏移量的差值，调用 moveArtifacts 完成实际移动，
     * 含边界约束。设置后 artifactOffsetX/Y 将更新为目标值（或约束后的值）。
     *
     * @param x 目标X偏移量。
     * @param y 目标Y偏移量。
     */
    void setArtifactOffset(qreal x, qreal y);

    /**
     * @brief 设置成品旋转到指定绝对角度（角度制）。
     *
     * 计算与当前旋转角度的差值，调用 rotateArtifacts 完成实际旋转，
     * 含边界约束。设置后 artifactRotation 将更新为目标值（或约束后的值）。
     *
     * @param angle 目标旋转角度（角度制）。
     */
    void setArtifactRotation(qreal angle);

    /**
     * @brief 设置成品亮度到指定绝对值。
     *
     * 计算与当前亮度偏移的差值，调用 adjustArtifactsBrightness 完成实际调整。
     *
     * @param value 目标亮度偏移值。
     */
    void setArtifactBrightness(int value);

    /**
     * @brief 获取当前成品透明度。
     *
     * @return 当前透明度值，范围 0.0~1.0。
     */
    qreal artifactOpacity() const { return m_artifactOpacity; }

    /**
     * @brief 获取当前成品旋转角度。
     *
     * @return 当前旋转角度（角度制）；未旋转时返回0.0。
     */
    qreal artifactRotation() const { return m_artifactParent ? m_artifactParent->rotation() : 0.0; }

    /**
     * @brief 获取当前成品亮度偏移值。
     *
     * @return 当前亮度偏移量；未调整时返回0。
     */
    int artifactBrightness() const { return m_artifactBrightness; }

    /**
     * @brief 获取当前成品X方向偏移量（场景坐标）。
     *
     * @return X偏移量；未移动时返回0.0。
     */
    qreal artifactOffsetX() const { return m_artifactOffsetX; }

    /**
     * @brief 获取当前成品Y方向偏移量（场景坐标）。
     *
     * @return Y偏移量；未移动时返回0.0。
     */
    qreal artifactOffsetY() const { return m_artifactOffsetY; }

    /**
     * @brief 检查是否已加载大板数据。
     *
     * @return true 已加载；false 未加载。
     */
    bool hasBoardData() const;

signals:
    /**
     * @brief 大板数据加载完成信号。
     *
     * @param success true 加载成功；false 加载失败。
     * @param error 错误信息（失败时有效）。
     */
    void boardLoaded(bool success, const QString &error);

    /**
     * @brief 大板数据被清除信号。
     */
    void boardCleared();

    /**
     * @brief 成品被选中信号。
     *
     * @param artifactId 成品ID。
     * @param artifactCode 成品编号。
     */
    void artifactSelected(qint64 artifactId, const QString &artifactCode);

private:
    /**
     * @brief 加载并显示背景图片。
     */
    void loadBackground();

    /**
     * @brief 绘制所有成品轮廓。
     */
    void drawArtifacts();

    /**
     * @brief 清除所有成品图元。
     */
    void clearArtifacts();

    /**
     * @brief 将板子坐标点映射到场景坐标。
     *
     * @param boardPoint 板子坐标点。
     * @return 场景坐标点。
     */
    QPointF mapBoardToScene(const QPointF &boardPoint) const;

    /**
     * @brief 将板子多边形顶点映射到场景多边形。
     *
     * @param vertices 顶点数组。
     * @return 场景坐标多边形。
     */
    QPolygonF mapBoardPolygonToScene(const QVector<vPoint2D> &vertices) const;

private:
    /**
     * @brief 关联的QGraphicsScene实例。
     */
    CustomGraphicsScene *m_scene;

    /**
     * @brief 关联的QGraphicsView实例，用于加载后自动适配显示。
     */
    QGraphicsView *m_view;

    /**
     * @brief 撤销栈。
     */
    QUndoStack *m_undoStack;

    /**
     * @brief 当前大板数据。
     */
    BoardData m_boardData;

    /**
     * @brief 背景图片。
     */
    QPixmap m_backgroundPixmap;

    /**
     * @brief 背景图片图元。
     */
    QGraphicsItem *m_backgroundItem;

    /**
     * @brief 成品图元列表。
     */
    QList<QGraphicsItem*> m_artifactItems;

    /**
     * @brief 成品标签图元列表。
     */
    QList<QGraphicsItem*> m_labelItems;

    /**
     * @brief 最后错误信息。
     */
    QString m_lastError;

    /**
     * @brief 成品轮廓画笔。
     */
    QPen m_artifactPen;

    /**
     * @brief 成品填充画刷。
     */
    QBrush m_artifactBrush;

    /**
     * @brief 是否显示成品标签。
     */
    bool m_showLabels;

    /**
     * @brief 板子尺寸（毫米）。
     */
    QSizeF m_boardSizeMM;

    /**
     * @brief 视图适配左侧边距（像素），用于避开左侧标尺。
     */
    int m_fitLeftMargin;

    /**
     * @brief 视图适配底侧边距（像素），用于避开底部标尺。
     */
    int m_fitBottomMargin;

    /**
     * @brief 是否正在进行整体拖动。
     *
     * true 表示正在拖动所有成品；false 表示空闲。
     */
    bool m_isGroupDragging;

    /**
     * @brief 整体拖动起始场景坐标。
     */
    QPointF m_dragStartScenePos;

    /**
     * @brief 整体拖动开始时父项的初始位置（场景坐标）。
     */
    QPointF m_parentStartPos;

    /**
     * @brief 是否允许成品拖拽，默认关闭。
     */
    bool m_artifactDragEnabled;

    /**
     * @brief 当前成品透明度，初始1.0（不透明）。
     */
    qreal m_artifactOpacity;

    /**
     * @brief 当前成品亮度偏移（HSV Value 增量），初始0表示未修改。
     */
    int m_artifactBrightness;

    /**
     * @brief 成品整体旋转/移动的不可见父项，初始nullptr。
     *
     * 使用普通 QGraphicsRectItem 代替 QGraphicsItemGroup，
     * 设置 ItemHasNoContents 使其不参与命中测试，
     * 子图元可正常选中，同时跟随父项旋转/移动变换。
     */
    QGraphicsRectItem *m_artifactParent;

    /**
     * @brief 成品整体旋转中心点，首次旋转时计算并缓存。
     *
     * 固定不变，避免多次旋转后因 boundingRect 变化导致中心偏移。
     */
    QPointF m_artifactRotationCenter;

    /**
     * @brief 成品X方向累计偏移量（场景坐标），初始0.0。
     */
    qreal m_artifactOffsetX;

    /**
     * @brief 成品Y方向累计偏移量（场景坐标），初始0.0。
     */
    qreal m_artifactOffsetY;
};

#endif // BOARDLOADHANDLER_H
