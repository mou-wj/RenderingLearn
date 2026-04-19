#include "Shader.h"
#include "RenderGraphBuilder.h"
#include "RHIApi.h"
#include <stdexcept>
#include <iostream>

namespace RenderCore {


Shader::Shader(const ShaderCompiledInitializer& initializer)
{
    Name = initializer.Type->Name;




}

// Destructor
Shader::~Shader()
{
    // Cleanup if necessary
}

// Compilation
bool Shader::Compile()
{
    try
    {
        // Create RHI Shader using RenderGraphBuilder
        //RHIShader = GRHIApi->CreateShader(ShaderSourceCode, ShaderType);
        if (!RHIShader)
        {
            throw std::runtime_error("Failed to create RHI shader for: " + Name);
        }


        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Shader compilation error: " << e.what() << std::endl;
        return false;
    }
}

void Shader::InitShaderBindings(const ShaderParametersMetadata* Metadata, const ShaderParameterAllocationMap& InParameterMap, std::string Prefix)
{
    if (!Metadata) return;

    // 1. 处理 UniformBuffer 整体绑定
    // 如果该 Metadata 被声明为 UniformBuffer (cbuffer)，它在物理表里通常有一个整体的 Slot
    if (Metadata->GetUseCase() == ShaderParametersMetadata::EUseCase::UniformBuffer)
    {
        // UniformBuffer 的名字在物理表中通常不带前缀，或者是顶级名字
        auto Alloc = InParameterMap.FindParameterAllocation(Metadata->GetStructName());
        if (Alloc.has_value())
        {
            ShaderParameterBindingInfo::ShaderResourceBinding UB;
            UB.BindSlot = Alloc->BaseIndex; // 对应的 register(bN)
            UB.BaseType = EShaderUniformBaseType::Unknown;
            UB.ArraySize = 1;

            // 将整个 UB 块存入资源绑定，Key 是结构体名
            Bindings.AddResourceBinding(Metadata->GetStructName(), UB);

            // 注意：如果这是一个 UB，我们通常不再递归处理它的内部成员的物理绑定，
            // 因为 UB 内部成员是通过 CPU 端的 Offset 进行内存拷贝的，而不是靠 GPU Slot。
            return;
        }
    }

    // 2. 遍历成员，处理嵌套结构体或独立变量 (Loose Parameters / Resources)
    for (const auto& Member : Metadata->GetMembers())
    {
        // 构造完整的反射路径名，例如 "View.Direction"
        std::string FullName = Prefix.empty() ? Member.Name : Prefix + Member.Name;

        if (Member.IsStruct())
        {
            // --- 递归处理嵌套结构体 ---
            // 传入 "Member名." 作为下一级的前缀
            InitShaderBindings(Member.StructMetadata, InParameterMap, FullName + ".");
        }
        else
        {
            // --- 处理叶子节点（基础变量或资源） ---
            auto Alloc = InParameterMap.FindParameterAllocation(FullName);

            if (Alloc.has_value())
            {
                const auto& PhysLoc = Alloc.value();

                if (Member.IsResource())
                {
                    // 处理 Texture, Sampler, UAV 等
                    ShaderParameterBindingInfo::ShaderResourceBinding Resource;
                    Resource.BindSlot = PhysLoc.BaseIndex; // register(t/s/u N)
                    Resource.BaseType = Member.BaseType;
                    Resource.ArraySize = (Member.NumElements > 0) ? Member.NumElements : 1;

                    Bindings.AddResourceBinding(FullName, Resource);
                }
                else
                {
                    // 处理 Loose Data (不在 UB 里的全局变量)
                    ShaderParameterBindingInfo::ShaderUniformBinding Uniform;
                    Uniform.BaseType = Member.BaseType;
                    Uniform.Offset = PhysLoc.BaseIndex; // 物理偏移或 Slot
                    Uniform.Size = PhysLoc.Size;

                    Bindings.AddUniformBinding(FullName, Uniform);
                }
            }
        }
    }
}


} // namespace RenderCore