#pragma once
#include <iostream>
#include <assert.h>
#define LogMessage(message) std::cout << message << " at :"<< __FILE__ << " : "<< __LINE__ << std::endl;
#define Log(message,condition) {if(!(condition))LogMessage(message);/*assert(condition);*/};
#define LogFunc Log(__func__,0)
#define LogInfo(message) LogMessage(message)
#define ASSERT(condition) assert((condition));
#define LogError(message) {LogMessage(message);LogFunc;ASSERT(0);};