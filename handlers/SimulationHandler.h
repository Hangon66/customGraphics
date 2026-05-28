#ifndef SIMULATIONHANDLER_H
#define SIMULATIONHANDLER_H

#include "IInteractionHandler.h"

#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QGraphicsEllipseItem>
#include <QTimer>
#include <QVector>
#include <QPen>

#include "../../structure.h"

/**
 * @brief 刀路模拟处理器，在 CustomGraphicsScene 上绘制并动画模拟刀路轨迹。
 *
 * 遵循 Handler 组合模式，不拦截鼠标/键盘事件（所有 handle* 返回 false），
 * 仅利用 Handler 架构进行统一管理。通过 setScene() 绑定外部场景后，
 * 可在该场景上叠加显示刀路线条、起终点标记和刀具指示器，并支持动画模拟。
 *
 * 图元 Z 值规划（确保显示在板材/成品之上）：
 * - 刀路线条: zValue = 100
 * - 起终点标记: zValue = 105
 * - 刀具指示器: zValue = 110
 */
class SimulationHandler : public AbstractInteractionHandler
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数。
     *
     * @param priority 处理器优先级，默认值 0（不拦截任何交互事件）。
     * @param parent 父 QObject 对象。
     */
    explicit SimulationHandler(int priority = 0, QObject *parent = nullptr);

    /**
     * @brief 获取处理器名称标识。
     *
     * @return 固定返回 "SimulationHandler"。
     */
    QString handlerName() const override;

    // ========== 场景绑定 ==========

    /**
     * @brief 绑定外部场景，后续所有图元将添加到此场景。
     *
     * 必须在 loadPath() 之前调用，否则路径无法绘制。
     *
     * @param scene 外部 QGraphicsScene 实例指针。
     */
    void setScene(QGraphicsScene *scene);

    // ========== 刀路加载与控制 ==========

    /**
     * @brief 加载刀路路径并在场景中绘制静态路径。
     *
     * 会先清理上一次的路径，然后根据路径点绘制线段（切割段灰色实线、空移段蓝色虚线）
     * 以及起点（绿色圆点）和终点（紫色圆点）标记，同时创建刀具指示器。
     *
     * @param path 切割点序列（机床坐标，mm）。
     */
    void loadPath(const QVector<CuttingPoint> &path);

    /**
     * @brief 开始模拟动画。
     *
     * 从路径起点开始，按定时器间隔逐点推进刀具指示器，
     * 并动态更新已完成段（绿色）、正在切割段（红色）和未完成段（灰色）。
     */
    void startSimulation();

    /**
     * @brief 暂停模拟动画。
     */
    void pauseSimulation();

    /**
     * @brief 停止模拟动画并重置到起始状态。
     */
    void stopSimulation();

    /**
     * @brief 获取模拟是否正在运行。
     *
     * @return true 定时器活跃，模拟运行中；false 已停止或暂停。
     */
    bool isRunning() const;

    /**
     * @brief 设置刀具指示器的视觉半径。
     *
     * @param radius 刀具指示器圆的半径（场景坐标单位）。
     */
    void setToolVisualRadius(double radius);

    /**
     * @brief 设置机床零点偏移量。
     *
     * CuttingPoint 坐标为机床坐标（含偏移），需减去此值再取反Y才能映射到场景坐标。
     * 应在 loadPath() 之前调用，与 CoordinateTransformer::setMachineOffset 保持一致。
     *
     * @param offsetX X 方向机床偏移（mm）。
     * @param offsetY Y 方向机床偏移（mm）。
     */
    void setMachineOffset(double offsetX, double offsetY);

    /**
     * @brief 清除当前加载的刀路，从场景中移除所有路径图元。
     */
    void clearPath();

signals:
    /**
     * @brief 模拟进度信号。
     *
     * @param current 当前已到达的路径点索引。
     * @param total 路径总点数减一（即最后一段的索引）。
     */
    void simulationProgress(int current, int total);

    /**
     * @brief 模拟完成信号。
     */
    void simulationFinished();

private slots:
    /**
     * @brief 定时器回调，推进模拟到下一个路径点。
     */
    void onTimerTick();

private:
    /**
     * @brief 在已绑定的场景上绘制静态路径线段和起终点标记。
     */
    void drawStaticPath();

    /**
     * @brief 根据当前刀具位置索引更新各段的颜色。
     *
     * @param currentIdx 当前刀具到达的路径点索引。
     */
    void updateSegmentColors(int currentIdx);

    /**
     * @brief 当前绑定的外部场景。
     */
    QGraphicsScene *m_scene = nullptr;

    /**
     * @brief 已加载的刀路路径点序列。
     */
    QVector<CuttingPoint> m_path;

    /**
     * @brief 动画定时器，驱动模拟逐帧推进。
     */
    QTimer m_timer;

    /**
     * @brief 当前模拟进行到的路径点索引。
     */
    int m_currentIdx = 0;

    /**
     * @brief 刀具指示器图元（红色实心圆）。
     */
    QGraphicsEllipseItem *m_toolIndicator = nullptr;

    /**
     * @brief 所有路径相关图元（用于统一清理）。
     */
    QVector<QGraphicsItem*> m_pathItems;

    /**
     * @brief 仅线段图元（按路径段顺序），用于动态换色。
     */
    QVector<QGraphicsLineItem*> m_lineItems;

    /**
     * @brief 刀具指示器的视觉半径（场景坐标单位）。
     */
    double m_toolVisualRadius = 4.0;

    /**
     * @brief 空移段画笔样式（蓝色虚线）。
     */
    QPen m_rapidPen;

    /**
     * @brief 机床零点 X 偏移（mm），用于机床坐标→场景坐标转换。
     */
    double m_machineOffsetX = 0.0;

    /**
     * @brief 机床零点 Y 偏移（mm），用于机床坐标→场景坐标转换。
     */
    double m_machineOffsetY = 0.0;
};

#endif // SIMULATIONHANDLER_H
