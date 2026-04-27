#include "BoardLoadHandler.h"
#include "../../boarddataloader.h"
#include "../view/CustomGraphicsScene.h"
#include "../view/ShapeMetadata.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QUndoStack>
#include <QDebug>
#include <QCoreApplication>
#include <QDir>
#include <QMouseEvent>

BoardLoadHandler::BoardLoadHandler(int priority, QObject *parent)
    : AbstractInteractionHandler(priority, parent)
    , m_scene(nullptr)
    , m_view(nullptr)
    , m_undoStack(nullptr)
    , m_backgroundItem(nullptr)
    , m_showLabels(true)
    , m_fitLeftMargin(0)
    , m_fitBottomMargin(0)
    , m_isGroupDragging(false)
{
    // 默认成品样式
    m_artifactPen = QPen(Qt::red, 2);
    m_artifactBrush = QBrush(QColor(255, 0, 0, 30));
}

BoardLoadHandler::~BoardLoadHandler()
{
    clearBoard();
}

QString BoardLoadHandler::handlerName() const
{
    return QStringLiteral("BoardLoadHandler");
}

bool BoardLoadHandler::loadBoardData(const QString &filePath)
{
    BoardDataLoader loader;
    if (!loader.loadFromFile(filePath)) {
        m_lastError = loader.lastError();
        emit boardLoaded(false, m_lastError);
        return false;
    }

    setBoardData(loader.boardData());
    return true;
}

void BoardLoadHandler::setBoardData(const BoardData &data)
{
    qDebug() << "[BoardLoadHandler] setBoardData 被调用";
    qDebug() << "[BoardLoadHandler] 板子数据: SN=" << data.sn 
             << "尺寸=" << data.length << "x" << data.wide 
             << "成品数量=" << data.artifacts.size();

    // 先清除旧数据
    clearBoard();

    m_boardData = data;
    m_boardSizeMM = QSizeF(data.length, data.wide);

    qDebug() << "[BoardLoadHandler] m_boardSizeMM 设置完成:" << m_boardSizeMM;

    // 加载背景图片
    loadBackground();

    // 绘制成品轮廓
    drawArtifacts();

    emit boardLoaded(true, QString());
}

void BoardLoadHandler::clearBoard()
{
    // 清空场景中所有图元（包括成品、标签、用户绘制矩形等）
    // 背景图片通过 drawBackground() 绘制，不是图元，不受影响
    if (m_scene) {
        m_scene->clear();
    }

    // 场景已清空，重置所有图元指针列表
    m_artifactItems.clear();
    m_labelItems.clear();
    m_backgroundItem = nullptr;  // 已被 clear() 释放

    m_backgroundPixmap = QPixmap();
    m_boardData = BoardData();
    m_boardSizeMM = QSizeF();

    // 清除场景的边界约束和背景
    if (m_scene) {
        m_scene->clearIndexes();
        m_scene->clearBackgroundPixmap();
    }

    emit boardCleared();
}

void BoardLoadHandler::setScene(CustomGraphicsScene *scene)
{
    // 如果已有场景，先清除数据
    if (m_scene && m_scene != scene) {
        clearBoard();
    }
    m_scene = scene;
}

void BoardLoadHandler::setView(QGraphicsView *view)
{
    m_view = view;
}

void BoardLoadHandler::setUndoStack(QUndoStack *undoStack)
{
    m_undoStack = undoStack;
}

void BoardLoadHandler::setFitMargins(int leftMargin, int bottomMargin)
{
    m_fitLeftMargin = qMax(0, leftMargin);
    m_fitBottomMargin = qMax(0, bottomMargin);
}

void BoardLoadHandler::setArtifactPen(const QPen &pen)
{
    m_artifactPen = pen;

    // 更新现有成品图元的画笔
    for (QGraphicsItem *item : m_artifactItems) {
        if (QGraphicsRectItem *rectItem = dynamic_cast<QGraphicsRectItem*>(item)) {
            rectItem->setPen(m_artifactPen);
        }
    }
}

void BoardLoadHandler::setArtifactBrush(const QBrush &brush)
{
    m_artifactBrush = brush;

    // 更新现有成品图元的画刷
    for (QGraphicsItem *item : m_artifactItems) {
        if (QGraphicsRectItem *rectItem = dynamic_cast<QGraphicsRectItem*>(item)) {
            rectItem->setBrush(m_artifactBrush);
        }
    }
}

void BoardLoadHandler::setShowLabels(bool show)
{
    if (m_showLabels == show) {
        return;
    }

    m_showLabels = show;

    // 更新标签可见性
    for (QGraphicsItem *item : m_labelItems) {
        item->setVisible(m_showLabels);
    }
}

void BoardLoadHandler::fitViewToBoard(QGraphicsView *view)
{
    if (!view || !m_scene) {
        return;
    }

    // 石板实际区域（原点左上角）
    QRectF boardRect(0, 0, m_backgroundPixmap.width(), m_backgroundPixmap.height());
    if (boardRect.isEmpty()) {
        return;
    }

    // 1. 自适应缩放：石板区域填满视口
    view->fitInView(boardRect, Qt::KeepAspectRatio);

    // 2. 左下角对齐到标尺内侧（扣除边距）
    // 当前石板左下角在视口中的位置
    QPointF currentVpBottomLeft = view->mapFromScene(boardRect.bottomLeft());

    // 目标视口位置：左侧边距、底部边距之上
    QPointF targetVpBottomLeft(
        m_fitLeftMargin,
        view->viewport()->height() - m_fitBottomMargin);

    // 计算需要的视口偏移
    QPointF deltaVp = targetVpBottomLeft - currentVpBottomLeft;

    // 将视口偏移转换为场景偏移
    QTransform transform = view->transform();
    QPointF sceneDelta(deltaVp.x() / transform.m11(),
                       deltaVp.y() / transform.m22());

    // 当前视口中心对应的场景坐标
    QPointF currentCenter = view->mapToScene(
        view->viewport()->width() / 2,
        view->viewport()->height() / 2);

    // 移动视口中心，使石板左下角对齐到目标位置
    view->centerOn(currentCenter - sceneDelta);
}

QSizeF BoardLoadHandler::boardSize() const
{
    if (!m_backgroundPixmap.isNull()) {
        return QSizeF(m_backgroundPixmap.width(), m_backgroundPixmap.height());
    }
    return QSizeF();
}

bool BoardLoadHandler::hasBoardData() const
{
    return m_boardData.isValid();
}

void BoardLoadHandler::loadBackground()
{
    if (!m_scene) {
        return;
    }

    // 构建图片的完整路径（相对于程序运行目录）
    QString imagePath = m_boardData.imageUrl;
    QString fullPath;

    // 如果已经是绝对路径，直接使用
    if (QDir::isAbsolutePath(imagePath)) {
        fullPath = imagePath;
    } else {
        // 相对于程序运行目录构建完整路径
        QString appDir = QCoreApplication::applicationDirPath();
        fullPath = QDir(appDir).filePath(imagePath);
    }

    // 加载图片（原图，竖向）
    QPixmap origPixmap(fullPath);

    if (origPixmap.isNull()) {
        qWarning() << "无法加载图片:" << fullPath << "（原始路径:" << imagePath << "），使用板子实际尺寸创建背景";
        
        // 使用板子的实际尺寸创建背景（旋转后的横向尺寸）
        int boardWidth = qMax(1, qRound(m_boardSizeMM.width()));    // 2827
        int boardHeight = qMax(1, qRound(m_boardSizeMM.height()));  // 1931
        origPixmap = QPixmap(boardWidth, boardHeight);
        origPixmap.fill(Qt::lightGray);
        
        qDebug() << "[BoardLoadHandler] 创建默认背景:" << boardWidth << "x" << boardHeight 
                 << "（基于板子尺寸" << m_boardSizeMM << "）";
        
        m_backgroundPixmap = origPixmap;
    } else {
        // 根据二维码角度旋转图片
        // 注意：QTransform::rotate是顺时针，而二维码angle是逆时针
        // angle = -87.24度表示逆时针87.24度，所以用rotate(-angle) = rotate(87.24)
        qreal rotationAngle = -m_boardData.qrCode.angle;  // 87.24度，顺时针
        
        QTransform transform;
        transform.rotate(rotationAngle);
        
        m_backgroundPixmap = origPixmap.transformed(transform, Qt::SmoothTransformation);
        
        qDebug() << "[BoardLoadHandler] 图片旋转:" << rotationAngle << "度"
                 << "原图尺寸:" << origPixmap.size()
                 << "旋转后尺寸:" << m_backgroundPixmap.size();
    }

    // 使用CustomGraphicsScene的背景功能
    if (m_scene) {
        m_scene->setBackgroundPixmap(m_backgroundPixmap);
    }
}

void BoardLoadHandler::drawArtifacts()
{
    if (!m_scene || m_boardData.artifacts.isEmpty()) {
        qDebug() << "[BoardLoadHandler] 无需绘制成品: scene=" << m_scene 
                 << "artifacts数量=" << m_boardData.artifacts.size();
        return;
    }

    qDebug() << "[BoardLoadHandler] 开始绘制" << m_boardData.artifacts.size() << "个成品";
    qDebug() << "[BoardLoadHandler] 板子尺寸(mm):" << m_boardSizeMM 
             << "背景尺寸:" << m_backgroundPixmap.size();

    clearArtifacts();

    for (const ArtifactInfo &artifact : m_boardData.artifacts) {
        // 使用 vertices（切割点位）进行绘制
        // vertices 包含成品在大板坐标系中的实际位置
        if (artifact.vertices.isEmpty()) {
            qDebug() << "[BoardLoadHandler] 成品" << artifact.artifactCode << "vertices为空，跳过绘制";
            continue;
        }

        // 将 vertices 顶点映射到场景坐标，并计算轴对齐矩形
        // 数据已旋转纠正，所有切割区域均为轴对齐矩形
        QPolygonF scenePolygon = mapBoardPolygonToScene(artifact.vertices);
        QRectF sceneRect = scenePolygon.boundingRect();

        // 创建矩形图元（轴对齐）
        QGraphicsRectItem *rectItem = new QGraphicsRectItem(sceneRect);
        rectItem->setPen(m_artifactPen);
        rectItem->setBrush(m_artifactBrush);
        rectItem->setFlag(QGraphicsItem::ItemIsSelectable, true);

        // 设置属性表，供 PropertyPanel 显示
        PropMap props;
        props["artifactCode"] = PropField(artifact.artifactCode, QStringLiteral("片名:"), PropType::Text, true, false);
        props["drawingMetaCode"] = PropField(artifact.drawingMetaCode, QStringLiteral("区域名:"), PropType::Text, true, false);
        int widthMM = qRound(artifact.width / 10.0);
        int heightMM = qRound(artifact.height / 10.0);
        props["width"] = PropField(widthMM, QStringLiteral("宽度(mm):"), PropType::Number, true, false);
        props["height"] = PropField(heightMM, QStringLiteral("高度(mm):"), PropType::Number, true, false);
        props["square"] = PropField(artifact.square, QStringLiteral("面积(m²):"), PropType::Number, true, false);
        props["rack"] = PropField(artifact.rack.isEmpty() ? QStringLiteral("手动") : artifact.rack,
                                   QStringLiteral("架号:"), PropType::Text, true, false);
        props["artifactId"] = PropField(artifact.id, QStringLiteral("成品ID:"), PropType::Int, true, false);
        // 预置几何属性为可见不可编辑，refreshGeometryProps 会自动更新值
        props["x"] = PropField(0.0, QStringLiteral("X:"), PropType::Number, true, false);
        props["y"] = PropField(0.0, QStringLiteral("Y:"), PropType::Number, true, false);
        rectItem->setData(ShapeMeta::Id, ShapeMeta::nextId());
        rectItem->setData(ShapeMeta::Category, QStringLiteral("Artifact"));
        rectItem->setData(ShapeMeta::ShapeType, ShapeMeta::Rect);
        rectItem->setData(ShapeMeta::Name, artifact.artifactCode);
        rectItem->setData(ShapeMeta::Props, QVariant::fromValue(props));

        m_scene->addItem(rectItem);
        m_artifactItems.append(rectItem);

        // 添加标签（全部居中显示，自适应字体大小，确保不超出矩形）
        if (m_showLabels) {
            QRectF bounds = sceneRect;
            QPointF center = bounds.center();

            // 最大可用区域（留出边距）
            qreal maxAllowedWidth = bounds.width() * 0.9;
            qreal maxAllowedHeight = bounds.height() * 0.8;

            // 初始字体大小
            qreal baseFontSize = qBound(12.0, qMin(bounds.width(), bounds.height()) / 5.0, 72.0);
            qreal scaleFactor = 1.0;

            // 准备标签文本
            QStringList labelTexts;
            labelTexts << artifact.artifactCode;
            if (artifact.width > 0 && artifact.height > 0) {
                int widthMM = qRound(artifact.width / 10.0);
                int heightMM = qRound(artifact.height / 10.0);
                labelTexts << QStringLiteral("%1×%2mm").arg(widthMM).arg(heightMM);
            }
            if (!artifact.rack.isEmpty()) {
                labelTexts << QStringLiteral("架:%1").arg(artifact.rack);
            }

            // 迭代缩小字体直到适合矩形
            QList<QGraphicsTextItem*> labels;
            qreal totalHeight = 0;
            qreal maxWidth = 0;
            qreal spacing = 0;
            int attempt = 0;
            const int maxAttempts = 10;

            while (attempt < maxAttempts) {
                // 清除之前的尝试
                for (auto *item : labels) {
                    delete item;
                }
                labels.clear();
                totalHeight = 0;
                maxWidth = 0;

                qreal currentFontSize = baseFontSize * scaleFactor;
                int padding = qRound(currentFontSize / 6.0);
                spacing = qRound(currentFontSize / 4.0);

                // 创建标签
                for (int i = 0; i < labelTexts.size(); ++i) {
                    qreal fontSize = currentFontSize * (1.0 - i * 0.1);  // 逐行字体稍小
                    QString text = labelTexts[i];

                    QGraphicsTextItem *textItem = new QGraphicsTextItem(text);
                    textItem->setZValue(10);

                    // 根据索引设置不同颜色（与淡红色背景对比度高）
                    QString textColor;
                    if (i == 0) {
                        textColor = "#00008B";  // 深蓝色（编号）
                    } else if (i == 1) {
                        textColor = "#006400";  // 深绿色（尺寸）
                    } else {
                        textColor = "#4B0082";  // 深紫色（架号）
                    }

                    textItem->setHtml(QStringLiteral(
                        "<div style='color:%1; font-size:%2px; font-weight:%3;'>%4</div>")
                        .arg(textColor).arg(qRound(fontSize))
                        .arg(i == 0 ? "bold" : "normal").arg(text));
                    textItem->setVisible(m_showLabels);
                    labels.append(textItem);

                    totalHeight += textItem->boundingRect().height();
                    maxWidth = qMax(maxWidth, textItem->boundingRect().width());
                }
                totalHeight += spacing * (labels.size() - 1);

                // 检查是否适合矩形
                if (totalHeight <= maxAllowedHeight && maxWidth <= maxAllowedWidth) {
                    break;  // 适合，退出循环
                }

                // 缩小字体重试
                scaleFactor *= 0.8;
                attempt++;
            }

            // 计算起始Y位置（让所有标签整体垂直居中）
            qreal currentY = center.y() - totalHeight / 2;

            // 设置每个标签的位置（水平居中，垂直堆叠）
            for (int i = 0; i < labels.size(); ++i) {
                QGraphicsTextItem *textItem = labels[i];
                qreal itemWidth = textItem->boundingRect().width();
                qreal itemHeight = textItem->boundingRect().height();
                textItem->setPos(center.x() - itemWidth / 2, currentY);
                currentY += itemHeight + spacing;

                textItem->setData(1, i == 0 ? QStringLiteral("label") :
                                         i == 1 ? QStringLiteral("sizeLabel") :
                                                  QStringLiteral("rackLabel"));
                // 标签关联的成品ID存入PropMap
                PropMap labelProps;
                labelProps["artifactId"] = PropField(artifact.id, QStringLiteral("成品ID:"), PropType::Int, true, false);
                textItem->setData(ShapeMeta::Props, QVariant::fromValue(labelProps));
                m_scene->addItem(textItem);
                m_labelItems.append(textItem);
            }
        }
    }
}

void BoardLoadHandler::clearArtifacts()
{
    // 删除所有成品图元
    for (QGraphicsItem *item : m_artifactItems) {
        if (m_scene) {
            m_scene->removeItem(item);
        }
        delete item;
    }
    m_artifactItems.clear();

    // 删除所有标签图元
    for (QGraphicsItem *item : m_labelItems) {
        if (m_scene) {
            m_scene->removeItem(item);
        }
        delete item;
    }
    m_labelItems.clear();
}

QPointF BoardLoadHandler::mapBoardToScene(const QPointF &boardPoint) const
{
    if (m_boardSizeMM.width() <= 0 || m_boardSizeMM.height() <= 0) {
        qDebug() << "[BoardLoadHandler] 板子尺寸无效:" << m_boardSizeMM;
        return boardPoint;
    }

    // 场景尺寸（旋转后的横向尺寸）
    qreal sceneWidth = m_backgroundPixmap.width();
    qreal sceneHeight = m_backgroundPixmap.height();

    if (sceneWidth <= 0 || sceneHeight <= 0) {
        // 使用默认尺寸（基于旋转后的板子尺寸）
        sceneWidth = m_boardSizeMM.width();   // 2827
        sceneHeight = m_boardSizeMM.height(); // 1931
    }

    // 坐标单位是 0.1mm，转换为毫米
    // vertices坐标已经是正确的物理坐标，直接转换即可
    qreal mmX = boardPoint.x() / 10.0;
    qreal mmY = boardPoint.y() / 10.0;

    // 计算缩放比例（毫米到像素）
    qreal scaleX = sceneWidth / m_boardSizeMM.width();   // 2827 / 2827 = 1
    qreal scaleY = sceneHeight / m_boardSizeMM.height(); // 1931 / 1931 = 1

    // 场景坐标
    // 注意：需要Y轴翻转，因为切割坐标系的Y方向与Qt场景Y方向相反
    qreal sceneX = mmX * scaleX;
    qreal sceneY = sceneHeight - (mmY * scaleY);  // Y轴翻转

    qDebug() << "[BoardLoadHandler] 坐标映射:" 
             << "原始(0.1mm)=" << boardPoint 
             << "毫米=" << QPointF(mmX, mmY)
             << "场景=" << QPointF(sceneX, sceneY);

    return QPointF(sceneX, sceneY);
}

QPolygonF BoardLoadHandler::mapBoardPolygonToScene(const QVector<vPoint2D> &vertices) const
{
    QPolygonF polygon;
    for (const vPoint2D &pt : vertices) {
        polygon << mapBoardToScene(QPointF(pt.x, pt.y));
    }
    return polygon;
}

bool BoardLoadHandler::handleMousePress(QGraphicsView *view, QMouseEvent *event)
{
    if (!isEnabled() || m_artifactItems.isEmpty() || event->button() != Qt::LeftButton) {
        return false;
    }

    // 获取鼠标位置下的图元
    QGraphicsItem *item = view->itemAt(event->pos());
    if (!item) {
        return false;
    }

    // 判断是否为成品矩形或标签
    bool isArtifact = (item->data(ShapeMeta::Category).toString() == QStringLiteral("Artifact"));
    bool isLabel = (item->data(1).toString() == QStringLiteral("label") ||
                   item->data(1).toString() == QStringLiteral("sizeLabel") ||
                   item->data(1).toString() == QStringLiteral("rackLabel"));

    if (!isArtifact && !isLabel) {
        return false;
    }

    // 手动选中对应的成品矩形图元，触发 PropertyPanel 更新
    if (m_scene) {
        qint64 artifactId = item->data(ShapeMeta::Props).value<PropMap>().value("artifactId").toInt();
        m_scene->clearSelection();
        if (isArtifact) {
            item->setSelected(true);
        } else {
            // 点击标签时，选中对应的成品矩形
            for (QGraphicsItem *artifactItem : m_artifactItems) {
                qint64 id = artifactItem->data(ShapeMeta::Props).value<PropMap>().value("artifactId").toInt();
                if (id == artifactId) {
                    artifactItem->setSelected(true);
                    break;
                }
            }
        }
    }

    // 进入整体拖动模式
    m_isGroupDragging = true;
    m_dragStartScenePos = view->mapToScene(event->pos());

    // 记录所有成品和标签的初始位置
    m_artifactStartPositions.clear();
    for (QGraphicsItem *artifactItem : m_artifactItems) {
        m_artifactStartPositions.append(artifactItem->pos());
    }

    m_labelStartPositions.clear();
    for (QGraphicsItem *labelItem : m_labelItems) {
        m_labelStartPositions.append(labelItem->pos());
    }

    return true;
}

bool BoardLoadHandler::handleMouseMove(QGraphicsView *view, QMouseEvent *event)
{
    if (!m_isGroupDragging) {
        return false;
    }

    // 计算场景坐标偏移量
    QPointF currentScenePos = view->mapToScene(event->pos());
    QPointF delta = currentScenePos - m_dragStartScenePos;

    // 应用边界约束：确保所有成品图元不超出石板区域
    if (m_scene && m_scene->hasBoundaryConstraint()) {
        QRectF boundary = m_scene->boundaryConstraint();

        // 计算所有成品图元移动后的总包围盒
        QRectF combinedRect;
        for (int i = 0; i < m_artifactItems.size() && i < m_artifactStartPositions.size(); ++i) {
            QGraphicsRectItem *rectItem = dynamic_cast<QGraphicsRectItem*>(m_artifactItems[i]);
            if (rectItem) {
                QPointF newPos = m_artifactStartPositions[i] + delta;
                QRectF itemRect = rectItem->rect().translated(newPos);
                combinedRect = combinedRect.isNull() ? itemRect : combinedRect.united(itemRect);
            }
        }

        // 限制偏移量，使包围盒不超出边界
        if (!combinedRect.isNull()) {
            if (combinedRect.left() < boundary.left()) {
                delta.setX(delta.x() + (boundary.left() - combinedRect.left()));
            }
            if (combinedRect.right() > boundary.right()) {
                delta.setX(delta.x() - (combinedRect.right() - boundary.right()));
            }
            if (combinedRect.top() < boundary.top()) {
                delta.setY(delta.y() + (boundary.top() - combinedRect.top()));
            }
            if (combinedRect.bottom() > boundary.bottom()) {
                delta.setY(delta.y() - (combinedRect.bottom() - boundary.bottom()));
            }
        }
    }

    // 同步移动所有成品图元
    for (int i = 0; i < m_artifactItems.size() && i < m_artifactStartPositions.size(); ++i) {
        m_artifactItems[i]->setPos(m_artifactStartPositions[i] + delta);
    }

    // 同步移动所有标签图元
    for (int i = 0; i < m_labelItems.size() && i < m_labelStartPositions.size(); ++i) {
        m_labelItems[i]->setPos(m_labelStartPositions[i] + delta);
    }

    // 通知属性面板实时刷新选中成品的坐标
    if (m_scene) {
        QList<QGraphicsItem*> selected = m_scene->selectedItems();
        if (!selected.isEmpty()) {
            emit m_scene->itemMoving(selected.first());
        }
    }

    return true;
}

bool BoardLoadHandler::handleMouseRelease(QGraphicsView *view, QMouseEvent *event)
{
    if (!m_isGroupDragging) {
        return false;
    }

    // 结束整体拖动
    m_isGroupDragging = false;
    m_artifactStartPositions.clear();
    m_labelStartPositions.clear();

    Q_UNUSED(view)
    Q_UNUSED(event)
    return true;
}
