#pragma once
#include <iostream>
#include <assert.h>
#define Log(message,condition) {if(!(condition))std::cout << message << " at :"<< __FILE__ << " : "<< __LINE__ << std::endl;/*assert(condition);*/};
#define LogFunc(condition) Log(__func__,condition)
#define LogInfo(message) {std::cout << message << " at :"<< __FILE__ << " : "<< __LINE__ << std::endl;}
#define ASSERT(condition) assert((condition));