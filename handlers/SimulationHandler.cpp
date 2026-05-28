#include "SimulationHandler.h"

#include <QPen>
#include <QBrush>
#include <QColor>
#include <QLineF>
#include <QDebug>

SimulationHandler::SimulationHandler(int priority, QObject *parent)
    : AbstractInteractionHandler(priority, parent)
{
    // 空移段画笔样式：蓝色虚线
    m_rapidPen = QPen(QColor(0, 160, 255), 1.5);
    m_rapidPen.setStyle(Qt::DashLine);

    // 定时器间隔 500ms
    m_timer.setInterval(500);
    connect(&m_timer, &QTimer::timeout, this, &SimulationHandler::onTimerTick);
}

QString SimulationHandler::handlerName() const
{
    return QStringLiteral("SimulationHandler");
}

void SimulationHandler::setScene(QGraphicsScene *scene)
{
    m_scene = scene;
}

void SimulationHandler::setToolVisualRadius(double radius)
{
    m_toolVisualRadius = radius;
}

void SimulationHandler::setMachineOffset(double offsetX, double offsetY)
{
    m_machineOffsetX = offsetX;
    m_machineOffsetY = offsetY;
}

void SimulationHandler::loadPath(const QVector<CuttingPoint> &path)
{
    clearPath();
    m_path = path;
    m_currentIdx = 0;

    if (m_path.isEmpty() || !m_scene) {
        qDebug() << __FUNCTION__ << "路径为空或场景未绑定，跳过绘制";
        return;
    }

    drawStaticPath();

    // 创建刀具指示器（红色实心圆）
    double r = m_toolVisualRadius;
    m_toolIndicator = new QGraphicsEllipseItem(-r, -r, 2 * r, 2 * r);
    m_toolIndicator->setBrush(QBrush(Qt::red));
    m_toolIndicator->setPen(Qt::NoPen);
    m_toolIndicator->setZValue(110);
    m_scene->addItem(m_toolIndicator);

    // 刀具定位到第一个点（机床坐标 → 场景坐标：减去偏移，Y取反）
    const CuttingPoint &first = m_path.first();
    m_toolIndicator->setPos(first.x - m_machineOffsetX, -(first.y - m_machineOffsetY));

    // 初始：所有切割段为灰色
    updateSegmentColors(0);

    qDebug() << "SimulationHandler::loadPath 已加载"
             << m_path.size() << "个路径点";
}

void SimulationHandler::startSimulation()
{
    if (m_path.isEmpty() || m_timer.isActive() || !m_scene)
        return;

    m_currentIdx = 0;
    if (m_toolIndicator) {
        const CuttingPoint &first = m_path.first();
        m_toolIndicator->setPos(first.x - m_machineOffsetX, -(first.y - m_machineOffsetY));
        m_toolIndicator->setVisible(true);
    } else {
        double r = m_toolVisualRadius;
        m_toolIndicator = new QGraphicsEllipseItem(-r, -r, 2 * r, 2 * r);
        m_toolIndicator->setBrush(QBrush(Qt::red));
        m_toolIndicator->setPen(Qt::NoPen);
        m_toolIndicator->setZValue(110);
        m_scene->addItem(m_toolIndicator);
    }
    updateSegmentColors(0);
    m_timer.start();
}

void SimulationHandler::pauseSimulation()
{
    m_timer.stop();
}

void SimulationHandler::stopSimulation()
{
    m_timer.stop();
    m_currentIdx = 0;
    if (m_toolIndicator && !m_path.isEmpty()) {
        m_toolIndicator->setPos(m_path.first().x - m_machineOffsetX,
                                 -(m_path.first().y - m_machineOffsetY));
        m_toolIndicator->setVisible(true);
    }
    updateSegmentColors(0);
}

bool SimulationHandler::isRunning() const
{
    return m_timer.isActive();
}

void SimulationHandler::clearPath()
{
    m_timer.stop();
    m_currentIdx = 0;
    m_path.clear();
    m_lineItems.clear();

    if (m_scene) {
        for (auto *item : m_pathItems) {
            m_scene->removeItem(item);
            delete item;
        }
    }
    m_pathItems.clear();

    if (m_toolIndicator) {
        if (m_scene) {
            m_scene->removeItem(m_toolIndicator);
        }
        delete m_toolIndicator;
        m_toolIndicator = nullptr;
    }
}

void SimulationHandler::onTimerTick()
{
    if (m_path.isEmpty() || m_currentIdx >= m_path.size() - 1) {
        m_timer.stop();
        updateSegmentColors(m_path.size());
        emit simulationFinished();
        return;
    }

    m_currentIdx++;
    const CuttingPoint &pt = m_path.at(m_currentIdx);
    if (m_toolIndicator) {
        m_toolIndicator->setPos(pt.x - m_machineOffsetX, -(pt.y - m_machineOffsetY));
    }

    updateSegmentColors(m_currentIdx);
    emit simulationProgress(m_currentIdx, m_path.size() - 1);
}

void SimulationHandler::drawStaticPath()
{
    if (!m_scene || m_path.size() < 2)
        return;

    for (int i = 0; i < m_path.size() - 1; ++i) {
        const CuttingPoint &p1 = m_path[i];
        const CuttingPoint &p2 = m_path[i + 1];

        QPen pen;
        bool isCutting = (p2.actionType == ActionType::LineCut ||
                          p2.actionType == ActionType::Plunge);
        if (isCutting) {
            // 切割段：初始灰色实线
            pen.setColor(Qt::gray);
            pen.setWidthF(2.0);
            pen.setStyle(Qt::SolidLine);
        } else {
            // 空移段：蓝色虚线
            pen = m_rapidPen;
        }

        QGraphicsLineItem *line = m_scene->addLine(
            QLineF(p1.x - m_machineOffsetX, -(p1.y - m_machineOffsetY),
                   p2.x - m_machineOffsetX, -(p2.y - m_machineOffsetY)), pen);
        line->setData(0, i);          // 记录路径段索引
        line->setZValue(100);
        m_lineItems.append(line);
        m_pathItems.append(line);
    }

    // 起点标记（绿色圆点）
    const CuttingPoint &startPt = m_path.first();
    QGraphicsEllipseItem *startCircle = m_scene->addEllipse(
        startPt.x - m_machineOffsetX - 3, -(startPt.y - m_machineOffsetY) - 3, 6, 6,
        QPen(Qt::green), QBrush(Qt::green));
    startCircle->setZValue(105);
    m_pathItems.append(startCircle);

    // 终点标记（紫色圆点）
    const CuttingPoint &endPt = m_path.last();
    QGraphicsEllipseItem *endCircle = m_scene->addEllipse(
        endPt.x - m_machineOffsetX - 3, -(endPt.y - m_machineOffsetY) - 3, 6, 6,
        QPen(Qt::darkMagenta), QBrush(Qt::darkMagenta));
    endCircle->setZValue(105);
    m_pathItems.append(endCircle);
}

void SimulationHandler::updateSegmentColors(int currentIdx)
{
    for (int i = 0; i < m_lineItems.size(); ++i) {
        QGraphicsLineItem *line = m_lineItems[i];
        int segIndex = line->data(0).toInt();
        if (segIndex + 1 >= m_path.size())
            continue;

        const CuttingPoint &nextPt = m_path[segIndex + 1];
        bool isCutting = (nextPt.actionType == ActionType::LineCut ||
                          nextPt.actionType == ActionType::Plunge);

        if (!isCutting) {
            // 空移段始终保持蓝色虚线
            continue;
        }

        QPen pen = line->pen();
        if (i < currentIdx - 1) {
            // 已完成的切割段：绿色
            pen.setColor(Qt::green);
            pen.setStyle(Qt::SolidLine);
        } else if (i == currentIdx - 1) {
            // 正在切割的段：红色
            pen.setColor(Qt::red);
            pen.setStyle(Qt::SolidLine);
        } else {
            // 未切割的段：灰色
            pen.setColor(Qt::gray);
            pen.setStyle(Qt::SolidLine);
        }
        line->setPen(pen);
    }
}
