#ifndef GRAPHICSTESTWIDGET_H
#define GRAPHICSTESTWIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class GraphicsTestWidget;
}
QT_END_NAMESPACE

class CustomGraphicsView;
class CustomGraphicsScene;

/**
 * @brief 图形测试主窗口，集成自定义 View 和 Scene 进行交互验证。
 *
 * 作为应用程序的主窗口容器，内嵌 CustomGraphicsView 和 CustomGraphicsScene，
 * 并添加测试图元用于验证缩放、平移、框选等交互功能。
 */
class GraphicsTestWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数，初始化窗口并集成图形视图和场景。
     *
     * @param parent 父 QWidget 对象。
     */
    explicit GraphicsTestWidget(QWidget *parent = nullptr);
    ~GraphicsTestWidget() override;

private:
    /**
     * @brief 向场景中添加用于测试交互的示例图元。
     */
    void addTestItems();

    /**
     * @brief Qt Designer 生成的 UI 对象指针。
     */
    Ui::GraphicsTestWidget *ui;

    /**
     * @brief 自定义图形场景实例，管理所有图元和网格背景。
     */
    CustomGraphicsScene *m_scene;

    /**
     * @brief 自定义图形视图实例，承载交互处理器（Handler）。
     */
    CustomGraphicsView *m_view;
};
#endif // GRAPHICSTESTWIDGET_H
