#ifndef BACKGROUNDHANDLER_H
#define BACKGROUNDHANDLER_H

#include "IInteractionHandler.h"
#include <QPixmap>
#include <QRectF>

class QGraphicsPixmapItem;
class QGraphicsScene;

/**
 * @brief 背景图片加载与管理处理器。
 *
 * 负责加载、显示和管理场景的背景图片。
 * 支持单张图片或平铺模式，可设置背景透明度。
 * 不处理交互事件，仅提供背景管理功能。
 */
class BackgroundHandler : public AbstractInteractionHandler
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数。
     *
     * @param priority 处理器优先级，默认值 0（最低，不参与事件处理）。
     * @param parent 父 QObject 对象。
     */
    explicit BackgroundHandler(int priority = 0, QObject *parent = nullptr);

    /**
     * @brief 析构函数。
     */
    ~BackgroundHandler() override;

    /**
     * @brief 获取处理器名称标识。
     *
     * @return 固定返回 "BackgroundHandler"。
     */
    QString handlerName() const override;

    /**
     * @brief 加载背景图片。
     *
     * 从指定路径加载图片并设置为场景背景。
     * 如果场景已存在背景，将被替换。
     *
     * @param filePath 图片文件路径。
     * @return true 加载成功；false 加载失败（文件不存在或格式不支持）。
     */
    bool loadBackground(const QString &filePath);

    /**
     * @brief 设置背景图片（使用已有的 QPixmap）。
     *
     * 直接使用传入的 QPixmap 作为背景图片。
     *
     * @param pixmap 背景图片像素图。
     */
    void setBackground(const QPixmap &pixmap);

    /**
     * @brief 清除当前背景图片。
     *
     * 移除场景中的背景图元，释放相关资源。
     */
    void clearBackground();

    /**
     * @brief 设置背景图片是否平铺显示。
     *
     * @param tiled true 平铺显示；false 居中显示（默认）。
     */
    void setTiled(bool tiled);

    /**
     * @brief 获取背景是否平铺显示。
     *
     * @return true 平铺显示；false 居中显示。
     */
    bool isTiled() const;

    /**
     * @brief 设置背景透明度。
     *
     * @param opacity 透明度值，范围 [0.0, 1.0]，默认 1.0（完全不透明）。
     */
    void setOpacity(qreal opacity);

    /**
     * @brief 获取背景透明度。
     *
     * @return 当前透明度值。
     */
    qreal opacity() const;

    /**
     * @brief 获取背景图片的边界矩形。
     *
     * @return 背景图片在场景坐标中的边界矩形；
     *         如果未加载背景，返回空矩形。
     */
    QRectF backgroundRect() const;

    /**
     * @brief 判断是否已加载背景图片。
     *
     * @return true 已加载背景；false 未加载背景。
     */
    bool hasBackground() const;

    /**
     * @brief 获取原始背景图片。
     *
     * @return 背景图片的副本；如果未加载，返回空 QPixmap。
     */
    QPixmap backgroundPixmap() const;

    /**
     * @brief 设置关联的场景。
     *
     * BackgroundHandler 需要知道场景对象才能添加背景图元。
     * 通常在 CustomGraphicsView 中自动调用。
     *
     * @param scene QGraphicsScene 实例。
     */
    void setScene(QGraphicsScene *scene);

    /**
     * @brief 获取关联的场景。
     *
     * @return 关联的 QGraphicsScene 实例；未设置则返回 nullptr。
     */
    QGraphicsScene *scene() const;

signals:
    /**
     * @brief 背景图片加载完成信号。
     *
     * @param success true 加载成功；false 加载失败。
     */
    void backgroundLoaded(bool success);

    /**
     * @brief 背景图片被清除信号。
     */
    void backgroundCleared();

private:
    /**
     * @brief 更新背景图元的显示。
     *
     * 根据当前的平铺设置和透明度重新生成背景图元。
     */
    void updateBackgroundItem();

    /**
     * @brief 创建平铺背景图片。
     *
     * 根据原始图片创建平铺后的大图。
     *
     * @param tileSize 原始图片尺寸。
     * @param targetSize 目标区域尺寸。
     * @return 平铺后的图片。
     */
    QPixmap createTiledPixmap(const QSize &tileSize, const QSize &targetSize) const;

private:
    /**
     * @brief 关联的 QGraphicsScene 实例。
     */
    QGraphicsScene *m_scene;

    /**
     * @brief 背景图片图元。
     *
     * 添加到场景中的图元，用于显示背景图片。
     */
    QGraphicsPixmapItem *m_backgroundItem;

    /**
     * @brief 原始背景图片。
     */
    QPixmap m_originalPixmap;

    /**
     * @brief 是否平铺显示。
     */
    bool m_tiled;

    /**
     * @brief 背景透明度。
     */
    qreal m_opacity;
};

#endif // BACKGROUNDHANDLER_H
