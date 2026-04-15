#include "graphicstestwidget.h"
#include "ui_graphicstestwidget.h"
#include "view/CustomGraphicsView.h"
#include "view/CustomGraphicsScene.h"
#include "handlers/SceneConfig.h"
#include "handlers/BackgroundHandler.h"
#include "handlers/RulerHandler.h"
#include "handlers/DrawHandler.h"

#include <QVBoxLayout>
#include <QGraphicsRectItem>
#include <QPen>
#include <QBrush>
#include <QDebug>

GraphicsTestWidget::GraphicsTestWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GraphicsTestWidget)
    , m_scene(nullptr)
    , m_view(nullptr)
    , m_backgroundHandler(nullptr)
    , m_rulerHandler(nullptr)
    , m_drawHandler(nullptr)
{
    ui->setupUi(this);
    initStoneCuttingScene();
}

GraphicsTestWidget::~GraphicsTestWidget()
{
    delete ui;
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

    // 设置布局
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_view);

    // 连接信号槽
    connectHandlers();

    // 添加测试图元
    addTestItems();

    qDebug() << "石材切割场景初始化完成:"
             << "标尺精度=" << config.rulerUnitPixel << "px/"
             << config.rulerUnitName;
}

void GraphicsTestWidget::addTestItems()
{
    // 添加示例石板区域
    auto *stoneRect = m_scene->addRect(0, 0, 600, 400,
                                        QPen(Qt::darkGray, 2),
                                        QBrush(QColor(200, 200, 200, 50)));
    stoneRect->setFlag(QGraphicsItem::ItemIsSelectable);
    stoneRect->setData(0, "StoneArea");
    stoneRect->setData(2, "石板区域");

    // 添加已有切割区域示例
    auto *cutArea1 = m_scene->addRect(50, 50, 150, 100,
                                       QPen(QColor(0, 100, 200), 2),
                                       QBrush(QColor(100, 149, 237, 80)));
    cutArea1->setFlag(QGraphicsItem::ItemIsSelectable);
    cutArea1->setFlag(QGraphicsItem::ItemIsMovable);
    cutArea1->setData(0, "CutArea");
    cutArea1->setData(2, "切割区域_1");

    auto *cutArea2 = m_scene->addRect(250, 150, 200, 150,
                                       QPen(QColor(0, 100, 200), 2),
                                       QBrush(QColor(100, 149, 237, 80)));
    cutArea2->setFlag(QGraphicsItem::ItemIsSelectable);
    cutArea2->setFlag(QGraphicsItem::ItemIsMovable);
    cutArea2->setData(0, "CutArea");
    cutArea2->setData(2, "切割区域_2");

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
