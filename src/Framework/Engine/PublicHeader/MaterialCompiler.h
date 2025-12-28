#pragma once
#include "Material.h"
namespace Engine {
    // MaterialCompiler
    class ENGINE_API MaterialCompiler
    {
    public:
        virtual ~MaterialCompiler() = default;

        //  ‰»Î Material + permutation key  ‰≥ˆ Shader
        virtual Shader* Compile(const Material& mat, const MaterialShaderKey& key) = 0;
    };




}

