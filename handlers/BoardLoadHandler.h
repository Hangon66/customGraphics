#ifndef BOARDLOADHANDLER_H
#define BOARDLOADHANDLER_H

#include "IInteractionHandler.h"
#include "../../structure.h"
#include <QPixmap>
#include <QPolygonF>
#include <QPen>
#include <QBrush>

class QGraphicsScene;
class QGraphicsItem;
class QGraphicsRectItem;
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
    void setScene(QGraphicsScene *scene);

    /**
     * @brief 获取关联的场景。
     *
     * @return 关联的QGraphicsScene实例；未设置则返回nullptr。
     */
    QGraphicsScene *scene() const { return m_scene; }

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
    QGraphicsScene *m_scene;

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
};

#endif // BOARDLOADHANDLER_H
