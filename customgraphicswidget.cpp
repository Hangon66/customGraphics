#include "customgraphicswidget.h"
#include "ui_customgraphicswidget.h"
#include "view/CustomGraphicsView.h"
#include "view/CustomGraphicsScene.h"
#include "handlers/SceneConfig.h"
#include "handlers/BackgroundHandler.h"
#include "handlers/RulerHandler.h"
#include "handlers/DrawHandler.h"
#include "handlers/CollisionHandler.h"
#include "commands/ShapeCommands.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsRectItem>
#include <QPen>
#include <QBrush>
#include <QDebug>
#include <QPushButton>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QKeyEvent>
#include <QUndoStack>

CustomGraphicsWidget::CustomGraphicsWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CustomGraphicsWidget)
    , m_scene(nullptr)
    , m_view(nullptr)
    , m_backgroundHandler(nullptr)
    , m_rulerHandler(nullptr)
    , m_drawHandler(nullptr)
    , m_modeButton(nullptr)
    , m_modeLabel(nullptr)
    , m_undoButton(nullptr)
    , m_redoButton(nullptr)
{
    ui->setupUi(this);
    initStoneCuttingScene();
    initToolBar();
}

CustomGraphicsWidget::~CustomGraphicsWidget()
{
    delete ui;
}

void CustomGraphicsWidget::keyPressEvent(QKeyEvent *event)
{
    QWidget::keyPressEvent(event);
}

void CustomGraphicsWidget::toggleDrawSelectMode()
{
    if (m_drawHandler) {
        m_drawHandler->toggleDrawingMode();
        updateModeDisplay();
    }
}

void CustomGraphicsWidget::initStoneCuttingScene()
{
    // 获取石材切割场景配置
    SceneConfig config = SceneConfigFactory::createStoneCuttingConfig();

    // 创建场景
    m_scene = new CustomGraphicsScene(this);

    // 配置碰撞检测：仅启用矩形间碰撞
    CollisionConfig collisionConfig;
    collisionConfig.enableRectOnlyCollision();
    m_scene->setCollisionConfig(collisionConfig);
    m_scene->setCollisionEnabled(true);

    // 创建视图
    m_view = new CustomGraphicsView(this);
    m_view->setScene(m_scene);

    // 根据配置创建并添加 Handler
    QList<IInteractionHandler*> handlers = config.createHandlers();
    for (IInteractionHandler* handler : handlers) {
        m_view->addHandler(handler);

        // 保存特定 Handler 的引用以便后续使用
        if (auto* bgHandler = dynamic_cast<BackgroundHandler*>(handler)) {
            m_backgroundHandler = bgHandler;
            m_backgroundHandler->setScene(m_scene);
        }
        else if (auto* rulerHandler = dynamic_cast<RulerHandler*>(handler)) {
            m_rulerHandler = rulerHandler;
        }
        else if (auto* drawHandler = dynamic_cast<DrawHandler*>(handler)) {
            m_drawHandler = drawHandler;
            m_drawHandler->setScene(m_scene);
            // 设置撤销栈，支持撤销/重做
            m_drawHandler->setUndoStack(m_view->undoStack());
        }
    }

    // 连接撤销栈变化信号，更新按钮状态
    connect(m_view->undoStack(), &QUndoStack::canUndoChanged,
            this, &CustomGraphicsWidget::updateUndoRedoState);
    connect(m_view->undoStack(), &QUndoStack::canRedoChanged,
            this, &CustomGraphicsWidget::updateUndoRedoState);

    // 连接信号槽
    connectHandlers();

    // 添加测试图元
    addTestItems();

    qDebug() << "石材切割场景初始化完成:"
             << "标尺精度=" << config.rulerUnitPixel << "px/"
             << config.rulerUnitName;
}

void CustomGraphicsWidget::initToolBar()
{
    // 创建工具栏容器
    QWidget *toolBar = new QWidget(this);
    toolBar->setFixedHeight(40);
    toolBar->setStyleSheet("background-color: #f0f0f0; border-bottom: 1px solid #ccc;");

    QHBoxLayout *toolLayout = new QHBoxLayout(toolBar);
    toolLayout->setContentsMargins(10, 5, 10, 5);
    toolLayout->setSpacing(10);

    // 模式切换按钮
    m_modeButton = new QPushButton("绘制模式", toolBar);
    m_modeButton->setCheckable(true);
    m_modeButton->setChecked(true);
    m_modeButton->setFixedWidth(100);
    m_modeButton->setStyleSheet(
        "QPushButton { padding: 5px 10px; border-radius: 3px; }"
        "QPushButton:checked { background-color: #4CAF50; color: white; }"
        "QPushButton:!checked { background-color: #2196F3; color: white; }");
    connect(m_modeButton, &QPushButton::clicked, this, &CustomGraphicsWidget::toggleDrawSelectMode);

    // 撤销/重做按钮
    m_undoButton = new QPushButton("撤销", toolBar);
    m_undoButton->setFixedWidth(60);
    m_undoButton->setEnabled(false);
    m_undoButton->setToolTip("Ctrl+Z");
    m_undoButton->setStyleSheet(
        "QPushButton { padding: 5px 10px; border-radius: 3px; }"
        "QPushButton:enabled { background-color: #607D8B; color: white; }"
        "QPushButton:disabled { background-color: #ccc; color: #666; }");
    connect(m_undoButton, &QPushButton::clicked, this, &CustomGraphicsWidget::onUndo);

    m_redoButton = new QPushButton("重做", toolBar);
    m_redoButton->setFixedWidth(60);
    m_redoButton->setEnabled(false);
    m_redoButton->setToolTip("Ctrl+Y");
    m_redoButton->setStyleSheet(
        "QPushButton { padding: 5px 10px; border-radius: 3px; }"
        "QPushButton:enabled { background-color: #607D8B; color: white; }"
        "QPushButton:disabled { background-color: #ccc; color: #666; }");
    connect(m_redoButton, &QPushButton::clicked, this, &CustomGraphicsWidget::onRedo);

    // 模式状态标签
    m_modeLabel = new QLabel("当前: 绘制模式 (按 D 切换)", toolBar);
    m_modeLabel->setStyleSheet("color: #333;");

    toolLayout->addWidget(m_modeButton);
    toolLayout->addWidget(m_undoButton);
    toolLayout->addWidget(m_redoButton);
    toolLayout->addWidget(m_modeLabel);
    toolLayout->addStretch();

    // 帮助提示
    QLabel *helpLabel = new QLabel("滚轮缩放 | 中键平移 | 左键绘制/选择 | Delete删除 | Esc取消", toolBar);
    helpLabel->setStyleSheet("color: #666; font-size: 11px;");
    toolLayout->addWidget(helpLabel);

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(toolBar);
    mainLayout->addWidget(m_view);

    // 更新模式显示
    updateModeDisplay();
}

void CustomGraphicsWidget::updateModeDisplay()
{
    if (!m_drawHandler || !m_modeButton || !m_modeLabel) {
        return;
    }

    bool isDrawMode = m_drawHandler->isDrawingActive();

    m_modeButton->setChecked(isDrawMode);
    m_modeButton->setText(isDrawMode ? "绘制模式" : "选择模式");

    m_modeLabel->setText(isDrawMode ?
        "当前: 绘制模式 (按 D 切换)" :
        "当前: 选择模式 (按 D 切换)");

    // 确保视图有焦点以接收按键事件
    if (m_view) {
        m_view->setFocus();
    }
}
void CustomGraphicsWidget::addTestItems()
{
    // 方式1：使用场景背景图片功能（推荐）
    // 创建一个示例背景图片（实际使用时可从文件加载）
    QPixmap bgPixmap(600, 400);
    bgPixmap.fill(QColor(240, 240, 240));  // 浅灰色背景
    
    // 在背景上绘制一些纹理（模拟石材）
    QPainter painter(&bgPixmap);
    painter.setPen(QPen(QColor(200, 200, 200), 1));
    for (int i = 0; i < 600; i += 20) {
        painter.drawLine(i, 0, i, 400);
    }
    for (int j = 0; j < 400; j += 20) {
        painter.drawLine(0, j, 600, j);
    }
    painter.end();
    
    // 设置场景背景图片（自动设置边界约束和场景大小）
    m_scene->setBackgroundPixmap(bgPixmap);
    qDebug() << "设置背景图片，尺寸:" << bgPixmap.width() << "x" << bgPixmap.height();
    qDebug() << "边界约束:" << m_scene->boundaryConstraint();
    
    // 同步设置 DrawHandler 的边界约束
    if (m_drawHandler) {
        m_drawHandler->setBoundaryConstraint(m_scene->boundaryConstraint());
    }

    // 添加已有切割区域示例
    auto *cutArea1 = new LabeledRectItem(50, 50, 150, 100);
    cutArea1->setPen(QPen(QColor(0, 100, 200), 2));
    cutArea1->setBrush(QBrush(QColor(100, 149, 237, 80)));
    cutArea1->setFlag(QGraphicsItem::ItemIsSelectable);
    cutArea1->setFlag(QGraphicsItem::ItemIsMovable);
    cutArea1->setData(0, "CutArea");
    cutArea1->setData(2, "切割区域_1");
    cutArea1->setLabelText("切割区域_1");
    m_scene->addItem(cutArea1);

    auto *cutArea2 = new LabeledRectItem(250, 150, 200, 150);
    cutArea2->setPen(QPen(QColor(0, 100, 200), 2));
    cutArea2->setBrush(QBrush(QColor(100, 149, 237, 80)));
    cutArea2->setFlag(QGraphicsItem::ItemIsSelectable);
    cutArea2->setFlag(QGraphicsItem::ItemIsMovable);
    cutArea2->setData(0, "CutArea");
    cutArea2->setData(2, "切割区域_2");
    cutArea2->setLabelText("切割区域_2");
    m_scene->addItem(cutArea2);
}

void CustomGraphicsWidget::connectHandlers()
{
    // 连接绘制完成信号
    if (m_drawHandler) {
        connect(m_drawHandler, &DrawHandler::shapeCreated,
                this, [](QGraphicsItem* item, DrawHandler::ShapeType type, const QString& name) {
            Q_UNUSED(item)
            QString typeStr;
            switch (type) {
            case DrawHandler::ShapeType::Rect: typeStr = "矩形"; break;
            case DrawHandler::ShapeType::Line: typeStr = "线条"; break;
            default: typeStr = "未知"; break;
            }
            qDebug() << "绘制完成:" << name << "类型:" << typeStr;
        });

        // 连接模式切换信号，更新 UI 显示
        connect(m_drawHandler, &DrawHandler::drawingActiveChanged,
                this, [this](bool active) {
            Q_UNUSED(active)
            updateModeDisplay();
        });
    }

    // 连接背景加载信号
    if (m_backgroundHandler) {
        connect(m_backgroundHandler, &BackgroundHandler::backgroundLoaded,
                this, [](bool success) {
            qDebug() << "背景加载:" << (success ? "成功" : "失败");
        });
    }

    // 连接场景的图元移动信号，支持移动撤销
    if (m_scene) {
        connect(m_scene, &CustomGraphicsScene::itemMoved,
                this, [this](QGraphicsItem *item, const QPointF &oldPos, const QPointF &newPos) {
            // 创建移动命令并推送到撤销栈
            MoveShapeCommand *cmd = new MoveShapeCommand(item, oldPos, newPos);
            m_view->undoStack()->push(cmd);
            // 不增加计数器，因为 push 会自动调用 redo
        });
    }
}

void CustomGraphicsWidget::onUndo()
{
    if (m_view) {
        m_view->undo();
    }
}

void CustomGraphicsWidget::onRedo()
{
    if (m_view) {
        m_view->redo();
    }
}

void CustomGraphicsWidget::updateUndoRedoState()
{
    if (m_view) {
        if (m_undoButton) {
            m_undoButton->setEnabled(m_view->canUndo());
        }
        if (m_redoButton) {
            m_redoButton->setEnabled(m_view->canRedo());
        }
    }
}
