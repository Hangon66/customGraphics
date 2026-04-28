#ifndef RULERHANDLER_H
#define RULERHANDLER_H

#include "IInteractionHandler.h"
#include <QColor>
#include <QFont>

class QGraphicsScene;
class QGraphicsView;

/**
 * @brief 精度标尺处理器。
 *
 * 在视图边缘绘制刻度标尺，显示当前场景坐标和缩放比例。
 * 支持自定义精度（单位像素值）和单位名称。
 * 默认标尺位置为左下角（底部水平标尺 + 左侧垂直标尺）。
 * 主要用于石材切割等需要高精度测量的场景。
 */
class RulerHandler : public AbstractInteractionHandler
{
    Q_OBJECT

public:
    /**
     * @brief 标尺位置枚举。
     */
    enum class RulerPosition
    {
        Top,    ///< 顶部水平标尺
        Bottom, ///< 底部水平标尺
        Left,   ///< 左侧垂直标尺
        Both    ///< 底部和左侧都显示（默认）
    };

    /**
     * @brief 构造函数。
     *
     * @param unitPixel 一个刻度单位对应的像素值（精度）。
     * @param unitName 单位名称（如 "mm"、"cm"）。
     * @param priority 处理器优先级，默认值 5。
     * @param parent 父 QObject 对象。
     */
    explicit RulerHandler(double unitPixel = 1.0,
                          const QString &unitName = "px",
                          int priority = 5,
                          QObject *parent = nullptr);

    /**
     * @brief 析构函数。
     */
    ~RulerHandler() override;

    /**
     * @brief 获取处理器名称标识。
     *
     * @return 固定返回 "RulerHandler"。
     */
    QString handlerName() const override;

    /**
     * @brief 设置一个单位对应的像素值（精度）。
     *
     * 例如：设置为 10.0 表示 1 个刻度单位 = 10 像素。
     *
     * @param unitPixel 单位像素值，必须大于 0。
     */
    void setUnitPixel(double unitPixel);

    /**
     * @brief 获取一个单位对应的像素值。
     *
     * @return 当前单位像素值。
     */
    double unitPixel() const;

    /**
     * @brief 设置单位名称。
     *
     * @param name 单位名称（如 "mm"、"cm"、"m"）。
     */
    void setUnitName(const QString &name);

    /**
     * @brief 获取单位名称。
     *
     * @return 当前单位名称。
     */
    QString unitName() const;

    /**
     * @brief 设置标尺宽度（像素）。
     *
     * @param width 标尺宽度，默认 25 像素。
     */
    void setRulerWidth(int width);

    /**
     * @brief 获取标尺宽度。
     *
     * @return 当前标尺宽度。
     */
    int rulerWidth() const;

    /**
     * @brief 设置标尺背景颜色。
     *
     * @param color 背景颜色。
     */
    void setBackgroundColor(const QColor &color);

    /**
     * @brief 获取标尺背景颜色。
     *
     * @return 当前背景颜色。
     */
    QColor backgroundColor() const;

    /**
     * @brief 设置标尺刻度颜色。
     *
     * @param color 刻度线和文字颜色。
     */
    void setTickColor(const QColor &color);

    /**
     * @brief 获取标尺刻度颜色。
     *
     * @return 当前刻度颜色。
     */
    QColor tickColor() const;

    /**
     * @brief 设置标尺位置。
     *
     * @param position 标尺显示位置。
     */
    void setRulerPosition(RulerPosition position);

    /**
     * @brief 获取标尺位置。
     *
     * @return 当前标尺位置。
     */
    RulerPosition rulerPosition() const;

    /**
     * @brief 设置是否显示标尺。
     *
     * @param visible true 显示；false 隐藏。
     */
    void setVisible(bool visible);

    /**
     * @brief 判断标尺是否可见。
     *
     * @return true 可见；false 隐藏。
     */
    bool isVisible() const;

    /**
     * @brief 强制刷新标尺显示。
     *
     * 当场景内容变化后调用此方法可立即更新标尺。
     */
    void refresh();

    /**
     * @brief 设置刻度标签的小数精度。
     *
     * 影响标尺刻度标签显示的小数位数。默认值为 0，
     * 即刻度值显示为整数。
     *
     * @param precision 小数位数，必须 >= 0。
     */
    void setDecimalPrecision(int precision);

    /**
     * @brief 获取刻度标签的小数精度。
     *
     * @return 当前小数精度。
     */
    int decimalPrecision() const;

    /**
     * @brief 绘制标尺。
     *
     * 通常由视图的绘制事件自动调用。
     *
     * @param painter 绘制器。
     * @param view 关联的视图。
     */
    void paint(QPainter *painter, QGraphicsView *view);

private:
    /**
     * @brief 绘制水平标尺。
     *
     * @param painter 绘制器。
     * @param view 关联的视图。
     * @param rect 标尺绘制区域。
     */
    void paintHorizontalRuler(QPainter *painter, QGraphicsView *view, const QRect &rect);

    /**
     * @brief 绘制垂直标尺。
     *
     * @param painter 绘制器。
     * @param view 关联的视图。
     * @param rect 标尺绘制区域。
     */
    void paintVerticalRuler(QPainter *painter, QGraphicsView *view, const QRect &rect);

    /**
     * @brief 计算合适的刻度间隔。
     *
     * 根据当前缩放级别自动计算刻度间隔，确保刻度不会太密集或太稀疏。
     *
     * @param pixelsPerUnit 当前每单位像素数（考虑缩放）。
     * @return 建议的刻度间隔（单位数）。
     */
    double calculateTickInterval(double pixelsPerUnit) const;

    /**
     * @brief 格式化刻度标签。
     *
     * @param value 刻度值。
     * @return 格式化后的标签字符串。
     */
    QString formatTickLabel(double value) const;

private:
    /**
     * @brief 一个单位对应的像素值（精度）。
     */
    double m_unitPixel;

    /**
     * @brief 单位名称。
     */
    QString m_unitName;

    /**
     * @brief 标尺宽度（像素）。
     */
    int m_rulerWidth;

    /**
     * @brief 标尺背景颜色。
     */
    QColor m_backgroundColor;

    /**
     * @brief 标尺刻度颜色。
     */
    QColor m_tickColor;

    /**
     * @brief 标尺位置。
     */
    RulerPosition m_position;

    /**
     * @brief 标尺是否可见。
     */
    bool m_visible;

    /**
     * @brief 刻度字体。
     */
    QFont m_font;

    /**
     * @brief 刻度标签小数精度。
     *
     * 控制 formatTickLabel 输出的小数位数，默认 0。
     */
    int m_decimalPrecision;
};

#endif // RULERHANDLER_H
