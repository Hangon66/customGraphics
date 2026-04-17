#ifndef STONESLABITEM_H
#define STONESLABITEM_H

#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QUrl>
#include <QRectF>
#include <QString>

class QNetworkReply;

/**
 * @brief 石材板图元类。
 *
 * 表示一块石材板，包含图片、尺寸信息和元数据。
 * 作为切割区域的边界约束容器。
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
     * @brief 从网络URL加载石材图片。
     *
     * @param imageUrl 图片URL。
     * @param baseUrl 基础URL（可选）。
     */
    void loadFromUrl(const QString &imageUrl, const QString &baseUrl = QString());

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
     * 用于碰撞检测和绘制约束。
     *
     * @return 边界矩形。
     */
    QRectF slabBoundingRect() const;

    /**
     * @brief 检查点是否在石材边界内。
     *
     * @param scenePos 场景坐标点。
     * @return true 在边界内；false 在边界外。
     */
    bool containsScenePoint(const QPointF &scenePos) const;

    /**
     * @brief 检查矩形是否完全在石材边界内。
     *
     * @param sceneRect 场景坐标矩形。
     * @return true 完全在边界内；false 部分或完全在边界外。
     */
    bool containsSceneRect(const QRectF &sceneRect) const;

    /**
     * @brief 将矩形约束到石材边界内。
     *
     * 如果矩形超出边界，返回约束后的矩形。
     *
     * @param sceneRect 场景坐标矩形。
     * @return 约束后的矩形。
     */
    QRectF constrainToBoundary(const QRectF &sceneRect) const;

    /**
     * @brief 重写绘制方法，添加边界框和标签。
     */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    /**
     * @brief 获取图元类型标识。
     *
     * @return "StoneSlab"
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

    /**
     * @brief 从网络URL加载图片。
     *
     * @param url 网络URL。
     */
    void loadFromNetwork(const QString &url);

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

    /**
     * @brief 是否正在加载图片。
     */
    bool m_isLoading;
};

#endif // STONESLABITEM_H
