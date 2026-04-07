// main.cpp
#include <iostream>
#include "TestBase.h"

int main(int argc, char** argv)
{
    std::string testName = "RHIShaderParameterTest"; // 默认
    if (argc > 1)
        testName = argv[1];

    auto test = Test::RenderTestRegistry::Create(testName);
    if (!test)
    {
        std::cerr << "Test not found: " << testName << std::endl;
        return -1;
    }

    try
    {
        test->Setup();
        test->Run();
        test->Teardown();
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Test failed with exception: " << ex.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "Test failed with unknown exception" << std::endl;
        return 1;
    }

    return 0;
}