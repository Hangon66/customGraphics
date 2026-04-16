#ifndef SHAPECOMMANDS_H
#define SHAPECOMMANDS_H

#include <QUndoCommand>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QList>

/**
 * @brief 创建图形命令。
 *
 * 封装图形的创建操作，支持撤销和重做。
 * 撤销时从场景移除图元，重做时添加图元到场景。
 */
class CreateShapeCommand : public QUndoCommand
{
public:
    /**
     * @brief 构造函数。
     *
     * @param scene 目标场景。
     * @param item 要创建的图元（所有权转移给命令）。
     * @param parent 父命令对象。
     */
    CreateShapeCommand(QGraphicsScene *scene, QGraphicsItem *item,
                       QUndoCommand *parent = nullptr);

    /**
     * @brief 析构函数。
     *
     * 如果图元未被添加到场景中，则负责删除图元。
     */
    ~CreateShapeCommand() override;

    /**
     * @brief 撤销操作，从场景移除图元。
     */
    void undo() override;

    /**
     * @brief 重做操作，将图元添加到场景。
     */
    void redo() override;

private:
    /**
     * @brief 目标场景。
     */
    QGraphicsScene *m_scene;

    /**
     * @brief 被创建的图元。
     */
    QGraphicsItem *m_item;

    /**
     * @brief 标记图元是否在场景中。
     */
    bool m_itemInScene;
};

/**
 * @brief 删除图形命令。
 *
 * 封装图形的删除操作，支持撤销和重做。
 * 撤销时恢复图元到场景，重做时从场景移除图元。
 */
class DeleteShapeCommand : public QUndoCommand
{
public:
    /**
     * @brief 构造函数。
     *
     * @param scene 目标场景。
     * @param items 要删除的图元列表（所有权转移给命令）。
     * @param parent 父命令对象。
     */
    DeleteShapeCommand(QGraphicsScene *scene, const QList<QGraphicsItem*> &items,
                       QUndoCommand *parent = nullptr);

    /**
     * @brief 析构函数。
     *
     * 如果图元未被添加到场景中，则负责删除图元。
     */
    ~DeleteShapeCommand() override;

    /**
     * @brief 撤销操作，恢复图元到场景。
     */
    void undo() override;

    /**
     * @brief 重做操作，从场景移除图元。
     */
    void redo() override;

private:
    /**
     * @brief 目标场景。
     */
    QGraphicsScene *m_scene;

    /**
     * @brief 被删除的图元列表。
     */
    QList<QGraphicsItem*> m_items;

    /**
     * @brief 标记图元是否在场景中。
     */
    bool m_itemsInScene;
};

/**
 * @brief 移动图形命令。
 *
 * 封装图形的移动操作，支持撤销和重做。
 * 记录移动前后的位置，撤销时恢复原位置。
 */
class MoveShapeCommand : public QUndoCommand
{
public:
    /**
     * @brief 构造函数。
     *
     * @param item 被移动的图元。
     * @param oldPos 移动前的位置。
     * @param newPos 移动后的位置。
     * @param alreadyMoved true 表示图元已在 newPos，redo() 时跳过移动（默认 true）。
     * @param parent 父命令对象。
     */
    MoveShapeCommand(QGraphicsItem *item, const QPointF &oldPos, const QPointF &newPos,
                     bool alreadyMoved = true, QUndoCommand *parent = nullptr);

    /**
     * @brief 撤销操作，恢复到原位置。
     */
    void undo() override;

    /**
     * @brief 重做操作，移动到新位置。
     */
    void redo() override;

private:
    /**
     * @brief 被移动的图元。
     */
    QGraphicsItem *m_item;

    /**
     * @brief 移动前的位置。
     */
    QPointF m_oldPos;

    /**
     * @brief 移动后的位置。
     */
    QPointF m_newPos;

    /**
     * @brief 标记首次 redo 是否跳过（图元已在目标位置）。
     */
    bool m_alreadyMoved;
};

#endif // SHAPECOMMANDS_H
