#pragma once

// shader compiler environment
struct ShaderCompilerEnvironment
{
    std::unordered_map<std::string, std::string> Macros;
};

// 所有 Shader 通用的最小参数
struct ShaderPermutationParameters
{
    uint32_t Platform;        // Vulkan / DX12 / Metal ...
    uint32_t PermutationId;   // Bitmask（功能组合）
    size_t GetHash() const {
		size_t hash = std::hash<uint32_t>()(Platform);
		hash ^= std::hash<uint32_t>()(PermutationId) << 1;
		return hash;
    }
};

using ShaderPermutationId = size_t;


// ------------------- 简化模板宏维度 -------------------
template<const char* MacroName>
struct FPermutationDimensionBool
{
    static constexpr const char* Name = MacroName;
    bool Value = false;
};

// ------------------- 简化 ShaderPermutationDomain -------------------
template<typename... Dimensions>
class ShaderPermutationDomain
{
public:
    std::tuple<Dimensions...> Values;

    ShaderPermutationDomain() = default;

    // 设置当前组合
    void SetCombination(std::array<bool, sizeof...(Dimensions)> Bools)
    {
        SetCombinationImpl(Bools, std::make_index_sequence<sizeof...(Dimensions)>{});
    }

    // 生成 PermutationId（打包成整数）
    ShaderPermutationId GetPermutationId() const
    {
        return GetPermutationIdImpl(std::make_index_sequence<sizeof...(Dimensions)>{});
    }

private:
    template<std::size_t... I>
    void SetCombinationImpl(const std::array<bool, sizeof...(Dimensions)>& Bools, std::index_sequence<I...>)
    {
        ((std::get<I>(Values).Value = Bools[I]), ...);
    }

    template<std::size_t... I>
    int32_t GetPermutationIdImpl(std::index_sequence<I...>) const
    {
        int32_t Id = 0;
        ((Id |= (std::get<I>(Values).Value ? (1 << I) : 0)), ...);
        return Id;
    }
};

/*
// ------------------- 宏名字符串 -------------------
constexpr char USE_VERTEX_COLOR[] = "USE_VERTEX_COLOR";
constexpr char USE_DITHERED_LOD[]  = "USE_DITHERED_LOD";

// ------------------- Shader 的 PermutationDomain -------------------
using PermutationDomain = TShaderPermutationDomain<
    FPermutationDimensionBool<USE_VERTEX_COLOR>,
    FPermutationDimensionBool<USE_DITHERED_LOD>
>;
    PermutationDomain Domain;

    Domain.SetCombination({true, false});
    std::cout << "PermutationId = " << Domain.GetPermutationId() << std::endl; // 1

*/