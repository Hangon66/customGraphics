#include "DragDropHandler.h"

#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsView>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QFile>

DragDropHandler::DragDropHandler(int priority, QObject *parent)
    : AbstractInteractionHandler(priority, parent)
    , m_scene(nullptr)
    , m_keepIconSize(true)
    , m_iconScale(1.0)
    , m_dragDropEnabled(true)
    , m_mimeType("application/x-device-type")
{
}

DragDropHandler::~DragDropHandler()
{
}

QString DragDropHandler::handlerName() const
{
    return QStringLiteral("DragDropHandler");
}

void DragDropHandler::setScene(QGraphicsScene *scene)
{
    m_scene = scene;
}

QGraphicsScene *DragDropHandler::scene() const
{
    return m_scene;
}

void DragDropHandler::registerDeviceType(const QString &typeId,
                                         const QString &typeName,
                                         const QPixmap &icon)
{
    DeviceTypeInfo info;
    info.name = typeName;
    info.icon = icon;
    m_deviceTypes[typeId] = info;
    emit deviceTypeRegistered(typeId);
}

bool DragDropHandler::registerDeviceType(const QString &typeId,
                                         const QString &typeName,
                                         const QString &iconPath)
{
    if (!QFile::exists(iconPath)) {
        return false;
    }

    QPixmap icon(iconPath);
    if (icon.isNull()) {
        return false;
    }

    registerDeviceType(typeId, typeName, icon);
    return true;
}

void DragDropHandler::unregisterDeviceType(const QString &typeId)
{
    if (m_deviceTypes.remove(typeId) > 0) {
        emit deviceTypeUnregistered(typeId);
    }
}

void DragDropHandler::clearDeviceTypes()
{
    QStringList typeIds = m_deviceTypes.keys();
    m_deviceTypes.clear();
    for (const QString &typeId : typeIds) {
        emit deviceTypeUnregistered(typeId);
    }
}

QStringList DragDropHandler::registeredDeviceTypes() const
{
    return m_deviceTypes.keys();
}

QString DragDropHandler::deviceTypeName(const QString &typeId) const
{
    return m_deviceTypes.value(typeId).name;
}

QPixmap DragDropHandler::deviceTypeIcon(const QString &typeId) const
{
    return m_deviceTypes.value(typeId).icon;
}

void DragDropHandler::setKeepIconSize(bool keepSize, qreal scale)
{
    m_keepIconSize = keepSize;
    if (!keepSize && scale > 0) {
        m_iconScale = scale;
    }
}

bool DragDropHandler::keepIconSize() const
{
    return m_keepIconSize;
}

qreal DragDropHandler::iconScale() const
{
    return m_iconScale;
}

void DragDropHandler::setDragDropEnabled(bool enabled)
{
    m_dragDropEnabled = enabled;
}

bool DragDropHandler::isDragDropEnabled() const
{
    return m_dragDropEnabled;
}

void DragDropHandler::setMimeType(const QString &mimeType)
{
    m_mimeType = mimeType;
}

QString DragDropHandler::mimeType() const
{
    return m_mimeType;
}

QString DragDropHandler::extractDeviceTypeId(const QMimeData *mimeData) const
{
    if (!mimeData || !mimeData->hasFormat(m_mimeType)) {
        return QString();
    }

    QByteArray data = mimeData->data(m_mimeType);
    return QString::fromUtf8(data);
}
