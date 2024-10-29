#pragma once
#include <iostream>
#include <assert.h>
#define Log(message,condition) {if(!(condition))std::cout << message << " at :"<< __FILE__ << " : "<< __LINE__ << std::endl;/*assert(condition);*/};
#define LogFunc Log(__func__,0)
#define LogInfo(message) Log(message,0)
#define ASSERT(condition) assert((condition));