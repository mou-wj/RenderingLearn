#pragma once
#include <iostream>
#include <assert.h>
#define Log(message,condition) do{ std::cout << message << " at :"<< __FILE__ << " : "<< __LINE__ << std::endl;assert(condition);} while(0);
#define LogFunc(condition) Log(__func__,condition)