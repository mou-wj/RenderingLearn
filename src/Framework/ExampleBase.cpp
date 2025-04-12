#include "ExampleBase.h"
#include <type_traits>
#include <iostream>


void ExampleBase::Run(ExampleBase* example)
{
	const std::type_info& info = typeid(*example);
	std::cout << "Run Example of Type : " << info.name() << std::endl;
	example->InitRaytrcingPipelineInfo();
	example->InitSubPassInfo();
	example->InitResourceInfos();
	example->InitSyncObjectNumInfo();
	example->Init();
	example->Loop();

}