#include "ShapeCommands.h"

// ==================== CreateShapeCommand 实现 ====================

CreateShapeCommand::CreateShapeCommand(CustomGraphicsScene *scene, QGraphicsItem *item,
                                       QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_scene(scene)
    , m_item(item)
    , m_itemInScene(false)
{
    setText(QObject::tr("创建图形"));
}

CreateShapeCommand::~CreateShapeCommand()
{
    // 如果图元不在场景中，由命令负责删除
    if (m_item && !m_itemInScene) {
        delete m_item;
    }
}

void CreateShapeCommand::undo()
{
    if (m_scene && m_item) {
        m_scene->removeItem(m_item);
        m_itemInScene = false;
    }
}

void CreateShapeCommand::redo()
{
    if (m_scene && m_item) {
        m_scene->addItem(m_item);
        m_itemInScene = true;
    }
}

// ==================== DeleteShapeCommand 实现 ====================

DeleteShapeCommand::DeleteShapeCommand(CustomGraphicsScene *scene, const QList<QGraphicsItem*> &items,
                                       QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_scene(scene)
    , m_items(items)
    , m_itemsInScene(true)
{
    setText(QObject::tr("删除图形"));
}

DeleteShapeCommand::~DeleteShapeCommand()
{
    // 如果图元不在场景中，由命令负责删除
    if (!m_itemsInScene) {
        qDeleteAll(m_items);
    }
}

void DeleteShapeCommand::undo()
{
    if (m_scene) {
        for (QGraphicsItem *item : m_items) {
            m_scene->addItem(item);
        }
        m_itemsInScene = true;
    }
}

void DeleteShapeCommand::redo()
{
    if (m_scene) {
        for (QGraphicsItem *item : m_items) {
            m_scene->removeItem(item);
        }
        m_itemsInScene = false;
    }
}

// ==================== MoveShapeCommand 实现 ====================

MoveShapeCommand::MoveShapeCommand(QGraphicsItem *item, const QPointF &oldPos, const QPointF &newPos,
                                   bool alreadyMoved, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_item(item)
    , m_oldPos(oldPos)
    , m_newPos(newPos)
    , m_alreadyMoved(alreadyMoved)
{
    setText(QObject::tr("移动图形"));
}

void MoveShapeCommand::undo()
{
    if (m_item) {
        m_item->setPos(m_oldPos);
        m_alreadyMoved = false;  // undo 后，下次 redo 需要移动
    }
}

void MoveShapeCommand::redo()
{
    if (m_item) {
        // 如果图元已在目标位置（首次 push），跳过移动
        if (!m_alreadyMoved) {
            m_item->setPos(m_newPos);
        }
        m_alreadyMoved = false;  // 后续 redo 都需要移动
    }
}
