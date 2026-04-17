#include "StoneSlabItem.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QDebug>

StoneSlabItem::StoneSlabItem(QGraphicsItem *parent)
    : QGraphicsPixmapItem(parent)
    , m_length(0)
    , m_width(0)
{
    initItem();
}

StoneSlabItem::~StoneSlabItem()
{
}

void StoneSlabItem::initItem()
{
    // 设置图元标识
    setData(0, "StoneSlab");

    // 设置 Z 值为最低层
    setZValue(-100);

    // 默认位置在原点
    setPos(0, 0);

    // 禁止选择和移动
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setFlag(QGraphicsItem::ItemIsMovable, false);
}

void StoneSlabItem::loadFromFile(const QString &filePath)
{
    QPixmap pixmap(filePath);
    if (!pixmap.isNull()) {
        setPixmap(pixmap);
        updateBoundingRect();
        qDebug() << "石材图片加载成功:" << filePath;
    } else {
        qWarning() << "无法加载图片:" << filePath;
    }
}

void StoneSlabItem::setSlabSize(double length, double width)
{
    m_length = length;
    m_width = width;
    updateBoundingRect();
}

double StoneSlabItem::slabLength() const
{
    return m_length;
}

double StoneSlabItem::slabWidth() const
{
    return m_width;
}

void StoneSlabItem::setSerialNumber(const QString &sn)
{
    m_serialNumber = sn;
    update();
}

QString StoneSlabItem::serialNumber() const
{
    return m_serialNumber;
}

void StoneSlabItem::setSlabName(const QString &name)
{
    m_slabName = name;
    update();
}

QString StoneSlabItem::slabName() const
{
    return m_slabName;
}

void StoneSlabItem::setMerchant(const QString &merchant)
{
    m_merchant = merchant;
    update();
}

QString StoneSlabItem::merchant() const
{
    return m_merchant;
}

QRectF StoneSlabItem::slabBoundingRect() const
{
    return m_boundingRect;
}

void StoneSlabItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // 先绘制图片
    QGraphicsPixmapItem::paint(painter, option, widget);

    // 绘制标签信息（右上角）
    if (!m_serialNumber.isEmpty() || !m_slabName.isEmpty()) {
        QString label;
        if (!m_slabName.isEmpty()) {
            label = m_slabName;
        }
        if (!m_serialNumber.isEmpty()) {
            if (!label.isEmpty()) {
                label += " - ";
            }
            label += m_serialNumber;
        }

        QFont font("Arial", 12, QFont::Bold);
        painter->setFont(font);
        QFontMetrics fm(font);

        // 标签位置：右上角外侧
        QPointF labelPos(m_boundingRect.right() - fm.horizontalAdvance(label) - 5,
                         m_boundingRect.top() - 5);

        // 绘制标签背景
        QRectF labelRect(labelPos.x() - 2, labelPos.y() - fm.height(),
                         fm.horizontalAdvance(label) + 4, fm.height() + 2);
        painter->fillRect(labelRect, QColor(255, 255, 255, 200));

        // 绘制标签文本
        painter->setPen(Qt::darkGray);
        painter->drawText(labelPos, label);
    }

    // 绘制尺寸标注（右下角）
    if (m_length > 0 && m_width > 0) {
        QString sizeLabel = QString("%1 x %2 mm").arg(static_cast<int>(m_length)).arg(static_cast<int>(m_width));
        QFont font("Arial", 10);
        painter->setFont(font);
        QFontMetrics fm(font);

        QPointF sizePos(m_boundingRect.right() - fm.horizontalAdvance(sizeLabel) - 5,
                        m_boundingRect.bottom() + fm.height() + 5);

        painter->setPen(Qt::gray);
        painter->drawText(sizePos, sizeLabel);
    }
}

int StoneSlabItem::type() const
{
    return UserType + 100;  // 自定义图元类型ID
}

void StoneSlabItem::updateBoundingRect()
{
    // 如果有图片，使用图片尺寸
    if (!pixmap().isNull()) {
        m_boundingRect = QRectF(0, 0, pixmap().width(), pixmap().height());
    }
    // 否则使用设置的尺寸
    else if (m_length > 0 && m_width > 0) {
        m_boundingRect = QRectF(0, 0, m_length, m_width);
    }
    // 默认空矩形
    else {
        m_boundingRect = QRectF();
    }

    prepareGeometryChange();
}
