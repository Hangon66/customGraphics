#ifndef DRAGDROPHANDLER_H
#define DRAGDROPHANDLER_H

#include "IInteractionHandler.h"
#include <QPixmap>
#include <QPointF>
#include <QString>
#include <QMap>

class QGraphicsScene;
class QGraphicsPixmapItem;
class QMimeData;

/**
 * @brief 设备图标拖放处理器。
 *
 * 支持从外部（如设备列表控件）拖放设备图标到场景中。
 * 主要用于楼层设备场景，实现设备图标的可视化布局。
 */
class DragDropHandler : public AbstractInteractionHandler
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数。
     *
     * @param priority 处理器优先级，默认为 15。
     * @param parent 父 QObject 对象。
     */
    explicit DragDropHandler(int priority = 15, QObject *parent = nullptr);

    /**
     * @brief 析构函数。
     */
    ~DragDropHandler() override;

    /**
     * @brief 获取处理器名称标识。
     *
     * @return 固定返回 "DragDropHandler"。
     */
    QString handlerName() const override;

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

    // ========== 设备注册 ==========

    /**
     * @brief 注册设备类型。
     *
     * 注册一个可拖放的设备类型及其图标。
     *
     * @param typeId 设备类型标识符。
     * @param typeName 设备类型名称（用于显示）。
     * @param icon 设备图标。
     */
    void registerDeviceType(const QString &typeId,
                            const QString &typeName,
                            const QPixmap &icon);

    /**
     * @brief 注册设备类型（使用图标文件路径）。
     *
     * @param typeId 设备类型标识符。
     * @param typeName 设备类型名称。
     * @param iconPath 图标文件路径。
     * @return true 注册成功；false 图标加载失败。
     */
    bool registerDeviceType(const QString &typeId,
                            const QString &typeName,
                            const QString &iconPath);

    /**
     * @brief 移除设备类型注册。
     *
     * @param typeId 设备类型标识符。
     */
    void unregisterDeviceType(const QString &typeId);

    /**
     * @brief 清除所有设备类型注册。
     */
    void clearDeviceTypes();

    /**
     * @brief 获取已注册的设备类型列表。
     *
     * @return 设备类型 ID 列表。
     */
    QStringList registeredDeviceTypes() const;

    /**
     * @brief 获取设备类型名称。
     *
     * @param typeId 设备类型标识符。
     * @return 设备类型名称；如果未注册，返回空字符串。
     */
    QString deviceTypeName(const QString &typeId) const;

    /**
     * @brief 获取设备类型图标。
     *
     * @param typeId 设备类型标识符。
     * @return 设备图标；如果未注册，返回空 QPixmap。
     */
    QPixmap deviceTypeIcon(const QString &typeId) const;

    // ========== 拖放配置 ==========

    /**
     * @brief 设置拖放时是否保持图标原始尺寸。
     *
     * @param keepSize true 保持原始尺寸；false 自适应缩放。
     * @param scale 缩放比例，仅在 keepSize 为 false 时有效。
     */
    void setKeepIconSize(bool keepSize, qreal scale = 1.0);

    /**
     * @brief 判断是否保持图标原始尺寸。
     *
     * @return true 保持原始尺寸；false 自适应缩放。
     */
    bool keepIconSize() const;

    /**
     * @brief 获取图标缩放比例。
     *
     * @return 当前缩放比例。
     */
    qreal iconScale() const;

    /**
     * @brief 设置是否启用拖放功能。
     *
     * @param enabled true 启用；false 禁用。
     */
    void setDragDropEnabled(bool enabled);

    /**
     * @brief 判断拖放功能是否启用。
     *
     * @return true 启用；false 禁用。
     */
    bool isDragDropEnabled() const;

    /**
     * @brief 设置 Mime 类型用于识别拖放数据。
     *
     * @param mimeType Mime 类型字符串。
     */
    void setMimeType(const QString &mimeType);

    /**
     * @brief 获取当前 Mime 类型。
     *
     * @return Mime 类型字符串。
     */
    QString mimeType() const;

signals:
    /**
     * @brief 设备拖放完成信号。
     *
     * @param item 创建的图元对象。
     * @param typeId 设备类型标识符。
     * @param typeName 设备类型名称。
     * @param position 放置位置（场景坐标）。
     */
    void deviceDropped(QGraphicsPixmapItem *item,
                       const QString &typeId,
                       const QString &typeName,
                       const QPointF &position);

    /**
     * @brief 设备类型注册信号。
     *
     * @param typeId 注册的设备类型标识符。
     */
    void deviceTypeRegistered(const QString &typeId);

    /**
     * @brief 设备类型注销信号。
     *
     * @param typeId 注销的设备类型标识符。
     */
    void deviceTypeUnregistered(const QString &typeId);

private:
    /**
     * @brief 设备类型信息结构体。
     */
    struct DeviceTypeInfo
    {
        QString name;   ///< 设备类型名称
        QPixmap icon;   ///< 设备图标
    };

    /**
     * @brief 从 Mime 数据中提取设备类型 ID。
     *
     * @param mimeData Mime 数据对象。
     * @return 设备类型 ID；如果无法提取，返回空字符串。
     */
    QString extractDeviceTypeId(const QMimeData *mimeData) const;

private:
    /**
     * @brief 关联的场景。
     */
    QGraphicsScene *m_scene;

    /**
     * @brief 设备类型注册表。
     */
    QMap<QString, DeviceTypeInfo> m_deviceTypes;

    /**
     * @brief 是否保持图标原始尺寸。
     */
    bool m_keepIconSize;

    /**
     * @brief 图标缩放比例。
     */
    qreal m_iconScale;

    /**
     * @brief 拖放功能是否启用。
     */
    bool m_dragDropEnabled;

    /**
     * @brief Mime 类型字符串。
     */
    QString m_mimeType;
};

#endif // DRAGDROPHANDLER_H
