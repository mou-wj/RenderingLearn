#include "RHIApi.h"


namespace RHI{
    static ERHIShaderPlatform GRHIShaderPlatform = ERHIShaderPlatform::Unknown;
    RHIApi* GRHIApi = nullptr;
    RHIApiInitHelper& RHIApiInitHelper::Instance()
    {
        static RHIApiInitHelper instance;
        return instance;
    }
    RHIApiInitHelper::~RHIApiInitHelper()
    {
        for (auto& pair : m_RHIApiMap)
        {
            delete pair.second; // 删除创建器
        }
        m_RHIApiMap.clear();
    }


    bool RHIApiInitHelper::InitGRHIApi(const ::std::string& apiName)
    {
        auto it = m_RHIApiMap.find(apiName);
        if (it != m_RHIApiMap.end())
        {
            GRHIApi = it->second->CreateRHIApi();
            if (GRHIApi)
            {
                return GRHIApi->Init();
            }
        }
        return false;
    }
    void RHIApiInitHelper::RegisterRHIApiCreator(const ::std::string& apiName, RHIApiCreator* creator)
    {
        m_RHIApiMap[apiName] = creator;
    }

    bool InitGRHIApi(const ::std::string& apiName)
    {
        return RHIApiInitHelper::Instance().InitGRHIApi(apiName);

    }


    RHIApi* GetGlobalRHIApi()
    {
        return GRHIApi;
    }

}