#include "SceneConfig.h"
#include "ZoomHandler.h"
#include "PanHandler.h"
#include "BackgroundHandler.h"
#include "RulerHandler.h"
#include "DrawHandler.h"
#include "DragDropHandler.h"

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
    config.rulerUnitPixel = 10.0;
    config.rulerUnitName = "mm";

    // 绘制配置
    config.enableDrawRect = true;
    config.enableDrawLine = false;
    config.enableNaming = true;
    config.defaultNamePrefix = "Cut";

    // 拖放配置
    config.enableDragDrop = false;

    // Handler 工厂
    config.createHandlers = []() -> QList<IInteractionHandler*> {
        QList<IInteractionHandler*> handlers;
        handlers << new ZoomHandler(100);
        handlers << new PanHandler(50);
        handlers << new BackgroundHandler();
        handlers << new RulerHandler(10.0, "mm");
        handlers << new DrawHandler(DrawHandler::DrawMode::Active, true, "Cut");
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

    // Handler 工厂
    config.createHandlers = []() -> QList<IInteractionHandler*> {
        QList<IInteractionHandler*> handlers;
        handlers << new ZoomHandler(100);
        handlers << new PanHandler(50);
        handlers << new BackgroundHandler();
        handlers << new DrawHandler(DrawHandler::DrawMode::Active, true, "Area");
        handlers << new DragDropHandler();
        return handlers;
    };

    return config;
}
