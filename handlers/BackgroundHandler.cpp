#include "BackgroundHandler.h"

#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QPainter>
#include <QFile>

BackgroundHandler::BackgroundHandler(int priority, QObject *parent)
    : AbstractInteractionHandler(priority, parent)
    , m_scene(nullptr)
    , m_backgroundItem(nullptr)
    , m_tiled(false)
    , m_opacity(1.0)
{
}

BackgroundHandler::~BackgroundHandler()
{
    clearBackground();
}

QString BackgroundHandler::handlerName() const
{
    return QStringLiteral("BackgroundHandler");
}

bool BackgroundHandler::loadBackground(const QString &filePath)
{
    if (!QFile::exists(filePath)) {
        emit backgroundLoaded(false);
        return false;
    }

    QPixmap pixmap(filePath);
    if (pixmap.isNull()) {
        emit backgroundLoaded(false);
        return false;
    }

    setBackground(pixmap);
    emit backgroundLoaded(true);
    return true;
}

void BackgroundHandler::setBackground(const QPixmap &pixmap)
{
    m_originalPixmap = pixmap;
    updateBackgroundItem();
}

void BackgroundHandler::clearBackground()
{
    if (m_backgroundItem && m_scene) {
        m_scene->removeItem(m_backgroundItem);
        delete m_backgroundItem;
        m_backgroundItem = nullptr;
    }
    m_originalPixmap = QPixmap();
    emit backgroundCleared();
}

void BackgroundHandler::setTiled(bool tiled)
{
    if (m_tiled != tiled) {
        m_tiled = tiled;
        updateBackgroundItem();
    }
}

bool BackgroundHandler::isTiled() const
{
    return m_tiled;
}

void BackgroundHandler::setOpacity(qreal opacity)
{
    opacity = qBound(0.0, opacity, 1.0);
    if (!qFuzzyCompare(m_opacity, opacity)) {
        m_opacity = opacity;
        updateBackgroundItem();
    }
}

qreal BackgroundHandler::opacity() const
{
    return m_opacity;
}

QRectF BackgroundHandler::backgroundRect() const
{
    if (m_backgroundItem) {
        return m_backgroundItem->boundingRect();
    }
    return QRectF();
}

bool BackgroundHandler::hasBackground() const
{
    return !m_originalPixmap.isNull();
}

QPixmap BackgroundHandler::backgroundPixmap() const
{
    return m_originalPixmap;
}

void BackgroundHandler::setScene(QGraphicsScene *scene)
{
    if (m_scene != scene) {
        // 切换场景时清除旧背景
        if (m_backgroundItem && m_scene) {
            m_scene->removeItem(m_backgroundItem);
            delete m_backgroundItem;
            m_backgroundItem = nullptr;
        }
        m_scene = scene;
        // 在新场景中重建背景
        if (m_scene && hasBackground()) {
            updateBackgroundItem();
        }
    }
}

QGraphicsScene *BackgroundHandler::scene() const
{
    return m_scene;
}

void BackgroundHandler::updateBackgroundItem()
{
    if (!m_scene || m_originalPixmap.isNull()) {
        return;
    }

    // 移除旧的背景图元
    if (m_backgroundItem) {
        m_scene->removeItem(m_backgroundItem);
        delete m_backgroundItem;
        m_backgroundItem = nullptr;
    }

    QPixmap displayPixmap = m_originalPixmap;

    // 处理平铺模式
    if (m_tiled && m_scene) {
        // 获取场景的合理大小用于平铺
        QRectF sceneRect = m_scene->sceneRect();
        if (sceneRect.isEmpty()) {
            sceneRect = QRectF(0, 0, 2000, 2000);
        }
        displayPixmap = createTiledPixmap(m_originalPixmap.size(), sceneRect.size().toSize());
    }

    // 处理透明度
    if (!qFuzzyCompare(m_opacity, 1.0)) {
        QPixmap transparentPixmap(displayPixmap.size());
        transparentPixmap.fill(Qt::transparent);
        
        QPainter painter(&transparentPixmap);
        painter.setOpacity(m_opacity);
        painter.drawPixmap(0, 0, displayPixmap);
        painter.end();
        
        displayPixmap = transparentPixmap;
    }

    // 创建背景图元并添加到场景最底层
    m_backgroundItem = m_scene->addPixmap(displayPixmap);
    m_backgroundItem->setZValue(-1000);  // 确保在最底层
    m_backgroundItem->setFlag(QGraphicsItem::ItemIsSelectable, false);
    m_backgroundItem->setFlag(QGraphicsItem::ItemIsMovable, false);
}

QPixmap BackgroundHandler::createTiledPixmap(const QSize &tileSize, const QSize &targetSize) const
{
    if (tileSize.isEmpty() || targetSize.isEmpty()) {
        return QPixmap();
    }

    QPixmap tiledPixmap(targetSize);
    tiledPixmap.fill(Qt::transparent);

    QPainter painter(&tiledPixmap);
    painter.setOpacity(m_opacity);

    // 平铺绘制
    for (int y = 0; y < targetSize.height(); y += tileSize.height()) {
        for (int x = 0; x < targetSize.width(); x += tileSize.width()) {
            painter.drawPixmap(x, y, m_originalPixmap);
        }
    }

    painter.end();
    return tiledPixmap;
}
