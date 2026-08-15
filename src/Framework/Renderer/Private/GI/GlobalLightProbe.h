#pragma once

// GlobalLightProbe
// ------------------------------------------------------------
// 这个文件负责生成全局漫反射光照探针。
//
// 设计目标：
//  - 在场景中布置一组低频漫反射探针，用于近似间接光照；
//  - 为动态对象、区域环境或局部遮蔽提供可采样的漫反射数据；
//  - 支持从体积采样和球谐/立方体贴图方式生成环境照明信息；
//  - 作为全局光照系统中的“低频照明近似层”。
//
// 预计包含的内容：
//  - GlobalLightProbeSettings：探针数量、分布方式、半径、采样数、卷积模式；
//  - LightProbeSamplePoint：采样位置和方向相关描述；
//  - LightProbeVolume：探针网络或体积网格结构；
//  - GlobalLightProbeBuilder：生成、更新、插值光照探针数据；
//  - SampleIrradiance()：根据位置和方向返回漫反射光照结果；
//  - UpdateProbes()：在场景变化后刷新探针；
//  - BuildFromScene()：从世界空间几何与光源构造探针。
//
// 说明：
//  - 当前阶段只定义职责和接口，不实现真正的探针计算；
//  - 重点是将它明确为“全局漫反射光照探针”模块，而不是传统的屏幕空间 GI；
//  - 后续可以结合 irradiance volume、SH coefficients 或 light probe grid 使用。
//
// TODO:
//  - 确定探针布局采用固定网格、自适应布局还是基于体积分段；
//  - 确定采样数据格式：SH、立方体贴图、4D 采样缓存或简化 irradiance 值；
//  - 与静态光照贴图和动态 GI 的数据交换方式。

namespace Renderer
{
    // 预留：全局漫反射光照探针的结构定义。
    // 例如：
    // struct GlobalLightProbeSettings
    // {
    //     uint32_t GridSizeX;
    //     uint32_t GridSizeY;
    //     uint32_t GridSizeZ;
    //     float ProbeSpacing;
    //     uint32_t SampleCount;
    // };
    //
    // class GlobalLightProbe
    // {
    // public:
    //     void BuildFromScene();
    //     void UpdateProbes();
    //     void SampleIrradiance(const Core::Float3& Position, Core::Float3& OutRadiance) const;
    // };
}

