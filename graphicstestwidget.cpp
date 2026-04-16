#include "graphicstestwidget.h"
#include "ui_graphicstestwidget.h"
#include "view/CustomGraphicsView.h"
#include "view/CustomGraphicsScene.h"
#include "handlers/SceneConfig.h"
#include "handlers/BackgroundHandler.h"
#include "handlers/RulerHandler.h"
#include "handlers/DrawHandler.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsRectItem>
#include <QPen>
#include <QBrush>
#include <QDebug>
#include <QPushButton>
#include <QLabel>
#include <QKeyEvent>

GraphicsTestWidget::GraphicsTestWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GraphicsTestWidget)
    , m_scene(nullptr)
    , m_view(nullptr)
    , m_backgroundHandler(nullptr)
    , m_rulerHandler(nullptr)
    , m_drawHandler(nullptr)
    , m_modeButton(nullptr)
    , m_modeLabel(nullptr)
{
    ui->setupUi(this);
    initStoneCuttingScene();
    initToolBar();
}

GraphicsTestWidget::~GraphicsTestWidget()
{
    delete ui;
}

void GraphicsTestWidget::keyPressEvent(QKeyEvent *event)
{
    // D 键切换绘制/选择模式
    if (event->key() == Qt::Key_D) {
        toggleDrawSelectMode();
        return;
    }

    QWidget::keyPressEvent(event);
}

void GraphicsTestWidget::toggleDrawSelectMode()
{
    if (m_drawHandler) {
        m_drawHandler->toggleDrawingMode();
        updateModeDisplay();
    }
}

void GraphicsTestWidget::initStoneCuttingScene()
{
    // 获取石材切割场景配置
    SceneConfig config = SceneConfigFactory::createStoneCuttingConfig();

    // 创建场景
    m_scene = new CustomGraphicsScene(this);

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
        }
    }

    // 连接信号槽
    connectHandlers();

    // 添加测试图元
    addTestItems();

    qDebug() << "石材切割场景初始化完成:"
             << "标尺精度=" << config.rulerUnitPixel << "px/"
             << config.rulerUnitName;
}

void GraphicsTestWidget::initToolBar()
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
    connect(m_modeButton, &QPushButton::clicked, this, &GraphicsTestWidget::toggleDrawSelectMode);

    // 模式状态标签
    m_modeLabel = new QLabel("当前: 绘制模式 (按 D 切换)", toolBar);
    m_modeLabel->setStyleSheet("color: #333;");

    toolLayout->addWidget(m_modeButton);
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

void GraphicsTestWidget::updateModeDisplay()
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
void GraphicsTestWidget::addTestItems()
{
    // 添加示例石板区域
    auto *stoneRect = new LabeledRectItem(0, 0, 600, 400);
    stoneRect->setPen(QPen(Qt::darkGray, 2));
    stoneRect->setBrush(QBrush(QColor(200, 200, 200, 50)));
    // stoneRect->setFlag(QGraphicsItem::ItemIsSelectable);
    stoneRect->setData(0, "StoneArea");
    stoneRect->setData(2, "石板区域");
    stoneRect->setLabelText("石板区域");
    stoneRect->setLabelColor(Qt::darkGray);
    m_scene->addItem(stoneRect);

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

    // 添加尺寸标注文本
    auto *dimText = m_scene->addText("600mm x 400mm", QFont("Arial", 12));
    dimText->setPos(200, 420);
    dimText->setDefaultTextColor(Qt::darkGray);
}

void GraphicsTestWidget::connectHandlers()
{
    // 连接绘制完成信号
    if (m_drawHandler) {
        connect(m_drawHandler, &DrawHandler::shapeCreated,
                this, [](QGraphicsItem* item, DrawHandler::ShapeType type, const QString& name) {
            qDebug() << "绘制完成:" << name
                     << "类型:" << (type == DrawHandler::ShapeType::Rect ? "矩形" : "线条");
        });
    }

    // 连接背景加载信号
    if (m_backgroundHandler) {
        connect(m_backgroundHandler, &BackgroundHandler::backgroundLoaded,
                this, [](bool success) {
            qDebug() << "背景加载:" << (success ? "成功" : "失败");
        });
    }
}
