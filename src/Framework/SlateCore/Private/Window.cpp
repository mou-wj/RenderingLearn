#include "Window.h"
namespace SlateCore {


    WindowSP WindowFactory::CreateWindowSP(int w, int h, const std::string& title) {
        return Creator(w, h, title);
    }


}