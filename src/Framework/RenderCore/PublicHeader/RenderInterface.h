#pragma once

#include <string>
#include <vector>


namespace RenderCore {

class RenderInterface
{
    public:
        virtual ~RenderInterface() = default;
        virtual void Init() = 0;
        virtual void Resize(uint32_t width, uint32_t height) = 0;
        virtual void Render() = 0;
        virtual void Update() = 0;
        virtual void Destroy() = 0;

};


}