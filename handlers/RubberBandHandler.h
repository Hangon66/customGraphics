#ifndef RUBBERBANDHANDLER_H
#define RUBBERBANDHANDLER_H

#include "IInteractionHandler.h"

#include <QPoint>
#include <Qt>

class QRubberBand;

/**
 * @brief 橡皮筋框选交互处理器。
 *
 * 响应鼠标左键拖拽操作，在视口上绘制矩形选框，
 * 释放时选中框内的所有图元。
 * 支持 Ctrl 修饰键进行追加选择模式。
 * 手动管理 QRubberBand 对象，不依赖 QGraphicsView 的内置 RubberBandDrag 模式。
 */
class RubberBandHandler : public AbstractInteractionHandler
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数。
     *
     * @param priority 处理器优先级，默认值 10（框选优先级通常最低）。
     * @param parent 父 QObject 对象。
     */
    explicit RubberBandHandler(int priority = 10, QObject *parent = nullptr);

    /**
     * @brief 处理鼠标按下事件，判断是否开始框选。
     *
     * 检查按下的按钮是否为配置的框选按钮，如果匹配则创建 QRubberBand 并开始框选。
     * 如果未按下追加选择修饰键（默认 Ctrl），会先清空已有选择。
     *
     * @param view 产生事件的 QGraphicsView 实例。
     * @param event 鼠标按下事件对象。
     * @return true 表示开始框选，事件被消费；false 表示不处理。
     */
    bool handleMousePress(QGraphicsView *view, QMouseEvent *event) override;

    /**
     * @brief 处理鼠标移动事件，更新选框大小。
     *
     * 在框选状态下，根据鼠标当前位置更新 QRubberBand 的几何区域。
     *
     * @param view 产生事件的 QGraphicsView 实例。
     * @param event 鼠标移动事件对象。
     * @return true 表示正在框选，事件被消费；false 表示不处理。
     */
    bool handleMouseMove(QGraphicsView *view, QMouseEvent *event) override;

    /**
     * @brief 处理鼠标释放事件，完成框选并选中图元。
     *
     * 将选框区域从视口坐标映射到场景坐标，
     * 调用 scene->setSelectionArea() 选中框内图元，
     * 然后销毁 QRubberBand。
     *
     * @param view 产生事件的 QGraphicsView 实例。
     * @param event 鼠标释放事件对象。
     * @return true 表示框选完成，事件被消费；false 表示不处理。
     */
    bool handleMouseRelease(QGraphicsView *view, QMouseEvent *event) override;

    /**
     * @brief 获取处理器名称标识。
     *
     * @return 固定返回 "RubberBandHandler"。
     */
    QString handlerName() const override;

    /**
     * @brief 设置触发框选的鼠标按钮。
     *
     * @param button 鼠标按钮，如 Qt::LeftButton。
     */
    void setSelectButton(Qt::MouseButton button);

    /**
     * @brief 设置选择判定模式。
     *
     * @param mode 选择模式，如 Qt::IntersectsItemShape 或 Qt::ContainsItemShape。
     */
    void setSelectionMode(Qt::ItemSelectionMode mode);

    /**
     * @brief 设置追加选择的键盘修饰键。
     *
     * 按住该修饰键框选时，不会清除已有选择，而是追加选中新图元。
     *
     * @param modifier 键盘修饰键，如 Qt::ControlModifier。
     */
    void setAddModifier(Qt::KeyboardModifier modifier);

private:
    /**
     * @brief 触发框选的鼠标按钮。
     *
     * 默认值为 Qt::LeftButton。
     */
    Qt::MouseButton m_selectButton;

    /**
     * @brief 选择判定模式。
     *
     * 默认值为 Qt::IntersectsItemShape，即图元与选框相交即被选中。
     */
    Qt::ItemSelectionMode m_selectionMode;

    /**
     * @brief 追加选择的键盘修饰键。
     *
     * 默认值为 Qt::ControlModifier。
     */
    Qt::KeyboardModifier m_modifierForAdd;

    /**
     * @brief 当前是否处于框选拖拽状态。
     */
    bool m_isSelecting;

    /**
     * @brief 框选起始点在视口中的坐标。
     */
    QPoint m_origin;

    /**
     * @brief 橡皮筋选框控件指针。
     *
     * 框选开始时创建，框选结束时销毁。
     * 父对象为视图的 viewport()，以确保坐标系正确。
     */
    QRubberBand *m_rubberBand;
};

#endif // RUBBERBANDHANDLER_H
