// main.cpp
#include <iostream>
#include "TestBase.h"

int main(int argc, char** argv)
{
    std::string testName = "RHIShaderParameter"; // 默认
    if (argc > 1)
        testName = argv[1];

    auto test = Test::RenderTestRegistry::Create(testName);
    if (!test)
    {
        std::cerr << "Test not found: " << testName << std::endl;
        return -1;
    }

    test->Setup();

    test->Run();

    test->Teardown();
    return 0;
}