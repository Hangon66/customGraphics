#ifndef STONESLABITEM_H
#define STONESLABITEM_H

#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QRectF>
#include <QString>

/**
 * @brief 石材板图元类。
 *
 * 表示一块石材板，包含图片、尺寸信息和元数据。
 * 提供标签和尺寸标注绘制功能。
 *
 * 注意：边界约束功能已迁移至 CustomGraphicsScene::setBackgroundPixmap()。
 * 此类主要用于需要元数据显示或图元级交互的场景。
 */
class StoneSlabItem : public QGraphicsPixmapItem
{
public:
    /**
     * @brief 构造函数。
     *
     * @param parent 父图元。
     */
    explicit StoneSlabItem(QGraphicsItem *parent = nullptr);

    /**
     * @brief 析构函数。
     */
    ~StoneSlabItem() override;

    /**
     * @brief 从本地文件加载石材图片。
     *
     * @param filePath 本地文件路径。
     */
    void loadFromFile(const QString &filePath);

    /**
     * @brief 设置石材尺寸（单位：mm）。
     *
     * @param length 长度（X方向）。
     * @param width 宽度（Y方向）。
     */
    void setSlabSize(double length, double width);

    /**
     * @brief 获取石材长度。
     *
     * @return 长度（mm）。
     */
    double slabLength() const;

    /**
     * @brief 获取石材宽度。
     *
     * @return 宽度（mm）。
     */
    double slabWidth() const;

    /**
     * @brief 设置石材编号。
     *
     * @param sn 编号字符串。
     */
    void setSerialNumber(const QString &sn);

    /**
     * @brief 获取石材编号。
     *
     * @return 编号字符串。
     */
    QString serialNumber() const;

    /**
     * @brief 设置石材名称。
     *
     * @param name 名称（如"纽西兰灰"）。
     */
    void setSlabName(const QString &name);

    /**
     * @brief 获取石材名称。
     *
     * @return 名称。
     */
    QString slabName() const;

    /**
     * @brief 设置商户信息。
     *
     * @param merchant 商户名称。
     */
    void setMerchant(const QString &merchant);

    /**
     * @brief 获取商户信息。
     *
     * @return 商户名称。
     */
    QString merchant() const;

    /**
     * @brief 获取石材边界矩形（场景坐标）。
     *
     * @return 边界矩形。
     */
    QRectF slabBoundingRect() const;

    /**
     * @brief 重写绘制方法，绘制标签和尺寸标注。
     */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    /**
     * @brief 获取图元类型标识。
     *
     * @return 自定义图元类型ID。
     */
    int type() const override;

private:
    /**
     * @brief 初始化图元属性。
     */
    void initItem();

    /**
     * @brief 更新边界矩形。
     */
    void updateBoundingRect();

private:
    /**
     * @brief 石材长度（mm）。
     */
    double m_length;

    /**
     * @brief 石材宽度（mm）。
     */
    double m_width;

    /**
     * @brief 石材编号。
     */
    QString m_serialNumber;

    /**
     * @brief 石材名称。
     */
    QString m_slabName;

    /**
     * @brief 商户名称。
     */
    QString m_merchant;

    /**
     * @brief 边界矩形（场景坐标）。
     */
    QRectF m_boundingRect;
};

#endif // STONESLABITEM_H
