#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "TextEditorScene.h"
#include "Engine.h"

int main( int argc, char * argv[] ){
    Engine engine(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO, 1280, 720, "TextEdit");
    engine.setFPS(120.0);
    engine.setCursor(true);

    std::string filePath = "";
    if(argc > 1){
        filePath = argv[1];
    }

    engine.getContext().scenes.changeScene(std::make_unique<TextEditorScene>(filePath));
    engine.run();
    return 0;
}
