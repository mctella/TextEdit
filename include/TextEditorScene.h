#ifndef TEXTEDITORSCENE_H
#define TEXTEDITORSCENE_H

#pragma once
#include <Game.h>
#include "Font.h"
#include "Texture.h"
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#include <sstream>
#include "dialog.h"

class TextEditorScene : public Game{
    private:
        Font editorFont;

        std::vector<std::string> lines;
        int cursRow = 0;
        int cursCol = 0;

        float blinkTimer = 0.0f;
        bool showCursor = true;
        bool isDirty = true;

        std::vector<Texture> lineTexture;
        std::string currentFile;

        int scrollOffset = 0;
        const int maxVisibleLines = 29;
    public:
        TextEditorScene(const std::string& filepath = "");

        void init(GameContext& context, Renderer& renderer) override;
        void update(float deltaTime, GameContext& context) override;
        void render(Renderer& renderer) override;

        void saveFile(const std::string& filename);
        void openFile(const std::string& filename);
};

#endif // TEXTEDITORSCENE_H
