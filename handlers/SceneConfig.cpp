#include "SceneConfig.h"
#include "ZoomHandler.h"
#include "PanHandler.h"
#include "BackgroundHandler.h"
#include "RulerHandler.h"
#include "DrawHandler.h"
#include "DragDropHandler.h"
#include "RubberBandHandler.h"

SceneConfig SceneConfigFactory::createStoneCuttingConfig()
{
    SceneConfig config;
    config.sceneName = "StoneCutting";
    config.description = "石材切割机可视化裁剪预览和编辑";

    // 背景配置
    config.enableBackground = true;
    config.backgroundTiled = false;

    // 标尺配置 - 高精度要求
    config.enableRuler = true;
    config.rulerUnitPixel = 1.0;  // 1像素 = 1mm，与标注一致
    config.rulerUnitName = "mm";

    // 绘制配置
    config.enableDrawRect = true;
    config.enableDrawLine = false;
    config.enableNaming = true;
    config.defaultNamePrefix = "成品";

    // 拖放配置
    config.enableDragDrop = false;

    // 碰撞配置 - 石材切割场景需要精确间距
    config.collisionMarginMM = 3.1;      // 碰撞边距3mm
    config.mmToSceneScale = 1.0;         // 1场景坐标=1mm

    // Handler 工厂
    config.createHandlers = [config]() -> QList<IInteractionHandler*> {
        QList<IInteractionHandler*> handlers;
        handlers << new ZoomHandler(100);
        handlers << new PanHandler(50);
        handlers << new BackgroundHandler();
        handlers << new RulerHandler(config.rulerUnitPixel, config.rulerUnitName);
        handlers << new DrawHandler(DrawHandler::DrawMode::Active, true, config.defaultNamePrefix);
        handlers << new RubberBandHandler(10);  // 框选优先级最低
        return handlers;
    };

    return config;
}

SceneConfig SceneConfigFactory::createFloorPlanConfig()
{
    SceneConfig config;
    config.sceneName = "FloorPlan";
    config.description = "楼层与设备预览";

    // 背景配置
    config.enableBackground = true;
    config.backgroundTiled = false;

    // 标尺配置 - 无精度要求
    config.enableRuler = false;
    config.rulerUnitPixel = 1.0;
    config.rulerUnitName = "px";

    // 绘制配置
    config.enableDrawRect = true;
    config.enableDrawLine = true;
    config.enableNaming = true;
    config.defaultNamePrefix = "Area";

    // 拖放配置
    config.enableDragDrop = true;

    // 碰撞配置 - 楼层场景无边距
    config.collisionMarginMM = 0.0;      // 无碰撞边距
    config.mmToSceneScale = 1.0;

    // Handler 工厂
    config.createHandlers = [config]() -> QList<IInteractionHandler*> {
        QList<IInteractionHandler*> handlers;
        handlers << new ZoomHandler(100);
        handlers << new PanHandler(50);
        handlers << new BackgroundHandler();
        handlers << new DrawHandler(DrawHandler::DrawMode::Active, true, config.defaultNamePrefix);
        handlers << new DragDropHandler();
        handlers << new RubberBandHandler(10);  // 框选优先级最低
        return handlers;
    };

    return config;
}
