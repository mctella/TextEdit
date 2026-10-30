#include "TextEditorScene.h"
#include <fstream>

TextEditorScene::TextEditorScene(const std::string& filepath) : currentFile(filepath) {}

void TextEditorScene::init(GameContext& context, Renderer& renderer){
    editorFont = Font("assets/font.ttf", 20);
    lines.push_back("");
    context.input.startTextInput();

    openFile(currentFile);
}

void TextEditorScene::update(float deltaTime, GameContext& context){
    std::string typed = context.input.getTypedText();
    if(!typed.empty()){
        lines[cursRow].insert(cursCol, typed);
        cursCol += typed.length();
        isDirty = true;
        context.input.clearTypedText();
    }

    blinkTimer += deltaTime;
    if(blinkTimer >= 0.5f){
        showCursor = !showCursor;
        blinkTimer = 0.0f;
    }

    if(context.input.wasKeyPressed(Key::Up)){
        if(cursRow > 0) cursRow--;
    }
    if(context.input.wasKeyPressed(Key::Down)){
        if(cursRow < static_cast<int>(lines.size() - 1)) cursRow++;
    }
    if(context.input.wasKeyPressed(Key::Left)){
        if(cursCol > 0) cursCol--;
    }
    if(context.input.wasKeyPressed(Key::Right)){
        if(cursCol < static_cast<int>(lines[cursRow].size())) cursCol++;
    }

    if(context.input.wasKeyPressed(Key::Return)){
        std::string currentLine = lines[cursRow];
        std::string remainder = currentLine.substr(cursCol);

        lines[cursRow] = currentLine.substr(0, cursCol);
        lines.insert(lines.begin() + cursRow + 1, remainder);

        cursRow++;
        cursCol = 0;
        isDirty = true;
    }

    if(context.input.wasKeyPressed(Key::Backspace)){
        if(cursCol > 0){
            lines[cursRow].erase(cursCol-1, 1);
            cursCol--;
            isDirty = true;
        }else if(cursRow > 0){
            cursCol = lines[cursRow - 1].size();
            lines[cursRow-1] += lines[cursRow];
            lines.erase(lines.begin() + cursRow);
            cursRow--;
            isDirty = true;
        }
    }

    if(context.input.wasKeyPressed(Key::Escape)){
        context.input.stopTextInput();
        context.input.setQuit(true);
    }

    if(context.input.isKeyDown(Key::LCTRL) && context.input.wasKeyPressed(Key::S)){
        std::string path = openNativeSaveDialog();
        if(!path.empty()){
            saveFile(path);
        }
    }


}

void TextEditorScene::render(Renderer& renderer){
    if(isDirty){
        lineTexture.clear();
        for(const auto& line : lines){
            if(line.empty()){
                lineTexture.push_back(Texture());
            }else{
                lineTexture.push_back(editorFont.createTextTexture(renderer, line, ColorCodes::WHITE));
            }
        }
        isDirty = false;
    }

    renderer.setColor(ColorCodes::BLACK);
    renderer.clear();

    float lineSpacing = 24.0f;
    float startX = 10.0f;
    float startY = 10.0f;

    renderer.setColor(ColorCodes::WHITE);
    for(size_t i = 0; i < lines.size(); i++){
        if(lines[i].empty()) continue;
        if(i >= lineTexture.size()) break;

        Rect line = {
            startX,
            startY + (i * lineSpacing),
            static_cast<float>(lineTexture[i].getWidth()),
            static_cast<float>(lineTexture[i].getHeight())
        };
        renderer.drawTexture(lineTexture[i], line);
    }

    int width = 0;
    int height = 0;

    TTF_SizeText(editorFont.getFont(), lines[cursRow].substr(0, cursCol).c_str(), &width, &height);

    renderer.setColor(ColorCodes::RED);
    if(showCursor){
        Rect cursRect = {
            startX + static_cast<float>(width),
            startY + static_cast<float>(height)*cursRow,
            3.0f,
            static_cast<float>(height),
        };
        renderer.drawRect(cursRect, true);
    };
}

void TextEditorScene::saveFile(const std::string& filename){}

void TextEditorScene::openFile(const std::string& filename){
    lines.clear();

    if(filename.empty()){
        lines.push_back("");
        isDirty = true;
        return;
    }

    std::ifstream inFile(filename);
    if(inFile.is_open()){
        std::string line;
        while(std::getline(inFile, line)){
            lines.push_back(line);
        }
        inFile.close();
    }

    if(lines.empty()){
        lines.push_back("");
    }

    cursRow = 0;
    cursCol = 0;
    isDirty = true;
}
