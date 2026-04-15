#include "graphicstestwidget.h"
#include "ui_graphicstestwidget.h"
#include "CustomGraphicsView.h"
#include "CustomGraphicsScene.h"

#include <QVBoxLayout>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QPen>
#include <QBrush>

GraphicsTestWidget::GraphicsTestWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GraphicsTestWidget)
    , m_scene(nullptr)
    , m_view(nullptr)
{
    ui->setupUi(this);

    m_scene = new CustomGraphicsScene(this);
    m_view = CustomGraphicsView::createDefaultView(this);
    m_view->setScene(m_scene);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_view);

    addTestItems();
}

GraphicsTestWidget::~GraphicsTestWidget()
{
    delete ui;
}

void GraphicsTestWidget::addTestItems()
{
    // 蓝色矩形
    auto *rect1 = m_scene->addRect(-100, -60, 200, 120,
                                    QPen(Qt::darkBlue, 2),
                                    QBrush(QColor(100, 149, 237)));
    rect1->setFlag(QGraphicsItem::ItemIsSelectable);
    rect1->setFlag(QGraphicsItem::ItemIsMovable);

    // 绿色矩形
    auto *rect2 = m_scene->addRect(150, 80, 120, 80,
                                    QPen(Qt::darkGreen, 2),
                                    QBrush(QColor(144, 238, 144)));
    rect2->setFlag(QGraphicsItem::ItemIsSelectable);
    rect2->setFlag(QGraphicsItem::ItemIsMovable);

    // 红色椭圆
    auto *ellipse1 = m_scene->addEllipse(-200, 100, 160, 100,
                                          QPen(Qt::darkRed, 2),
                                          QBrush(QColor(255, 127, 80)));
    ellipse1->setFlag(QGraphicsItem::ItemIsSelectable);
    ellipse1->setFlag(QGraphicsItem::ItemIsMovable);

    // 紫色椭圆
    auto *ellipse2 = m_scene->addEllipse(200, -150, 100, 100,
                                          QPen(QColor(128, 0, 128), 2),
                                          QBrush(QColor(221, 160, 221)));
    ellipse2->setFlag(QGraphicsItem::ItemIsSelectable);
    ellipse2->setFlag(QGraphicsItem::ItemIsMovable);

    // 文本标签
    auto *text = m_scene->addText("Handler Test", QFont("Arial", 16));
    text->setPos(-60, -120);
    text->setFlag(QGraphicsItem::ItemIsSelectable);
    text->setFlag(QGraphicsItem::ItemIsMovable);
    text->setDefaultTextColor(Qt::darkGray);
}
