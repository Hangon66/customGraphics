#ifndef SCENECONFIG_H
#define SCENECONFIG_H

#include <QString>
#include <QList>
#include <functional>
#include <memory>

class IInteractionHandler;

/**
 * @brief 场景配置描述结构体。
 *
 * 用于定义一个业务场景所需的 Handler 组合和配置参数。
 * 通过配置结构体而非继承来实现场景差异化，符合组合优于继承的设计原则。
 */
struct SceneConfig
{
    /**
     * @brief 场景名称标识。
     */
    QString sceneName;

    /**
     * @brief 场景描述信息。
     */
    QString description;

    // ========== 背景配置 ==========

    /**
     * @brief 是否启用背景图片。
     *
     * true 表示场景需要加载背景图片；
     * false 表示无背景图片。
     */
    bool enableBackground = false;

    /**
     * @brief 背景图片是否平铺显示。
     *
     * true 表示背景图片以平铺方式填充；
     * false 表示背景图片居中或拉伸显示。
     */
    bool backgroundTiled = false;

    /**
     * @brief 默认背景图片路径。
     */
    QString defaultBackgroundPath;

    // ========== 标尺配置 ==========

    /**
     * @brief 是否启用标尺。
     *
     * true 表示显示精度标尺；
     * false 表示不显示标尺。
     */
    bool enableRuler = false;

    /**
     * @brief 标尺单位对应的像素值（精度）。
     *
     * 例如：值为 10.0 表示 1 个单位刻度 = 10 像素。
     */
    double rulerUnitPixel = 1.0;

    /**
     * @brief 标尺单位名称。
     *
     * 例如："mm"、"cm"、"m" 等。
     */
    QString rulerUnitName = "px";

    // ========== 绘制配置 ==========

    /**
     * @brief 是否支持绘制矩形。
     */
    bool enableDrawRect = false;

    /**
     * @brief 是否支持绘制线条。
     */
    bool enableDrawLine = false;

    /**
     * @brief 是否支持为绘制的图形命名。
     */
    bool enableNaming = false;

    /**
     * @brief 默认图形名称前缀。
     */
    QString defaultNamePrefix = "Shape";

    // ========== 拖放配置 ==========

    /**
     * @brief 是否支持拖放设备图标。
     */
    bool enableDragDrop = false;

    /**
     * @brief 设备图标资源目录路径。
     */
    QString deviceIconPath;

    // ========== 碰撞配置 ==========

    /**
     * @brief 碰撞边距，单位：毫米。
     *
     * 碰撞检测时障碍物边界向四周扩展该值，使图元间保持指定间距。
     * 0.0 表示无边距（图元可紧贴）。默认 0.0。
     * 内部通过 mmToSceneScale 转换为场景坐标单位。
     */
    double collisionMarginMM = 0.0;

    /**
     * @brief 毫米到场景坐标的缩放因子。
     *
     * 用于碰撞边距的 mm 到场景坐标转换，与 rulerUnitPixel 含义一致：
     * 1mm 对应的场景坐标像素数。默认 1.0（即 1mm = 1场景单位）。
     */
    double mmToSceneScale = 1.0;

    // ========== Handler 工厂 ==========

    /**
     * @brief Handler 工厂函数类型。
     *
     * 返回该场景所需的所有 Handler 实例列表。
     */
    using HandlerFactory = std::function<QList<IInteractionHandler*>()>;

    /**
     * @brief 创建该场景所需的所有 Handler。
     *
     * 调用此函数将返回配置好的 Handler 实例列表，
     * 调用者负责管理返回的 Handler 对象生命周期。
     */
    HandlerFactory createHandlers;
};

/**
 * @brief 场景配置工厂类。
 *
 * 提供预定义的两种业务场景配置：
 * - 石材切割场景：高精度标尺、背景图片、矩形绘制命名
 * - 楼层设备场景：背景图片、矩形线条绘制、设备拖放
 */
class SceneConfigFactory
{
public:
    /**
     * @brief 创建石材切割场景配置。
     *
     * 场景特点：
     * - 启用背景图片（石板背景）
     * - 启用精度标尺
     * - 支持绘制矩形并命名
     * - 不支持线条绘制和设备拖放
     *
     * @return 配置好的 SceneConfig 实例。
     */
    static SceneConfig createStoneCuttingConfig();

    /**
     * @brief 创建楼层设备场景配置。
     *
     * 场景特点：
     * - 启用背景图片（楼层平面图）
     * - 无精度标尺
     * - 支持绘制矩形和线条并命名
     * - 支持拖放设备图标
     *
     * @return 配置好的 SceneConfig 实例。
     */
    static SceneConfig createFloorPlanConfig();
};

#endif // SCENECONFIG_H
