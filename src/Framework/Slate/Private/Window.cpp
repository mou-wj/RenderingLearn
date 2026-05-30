#include "Window.h"
namespace Slate {


    WindowSP WindowFactory::CreateWindowSP(int w, int h, const std::string& title) {
        return Creator(w, h, title);
    }


}