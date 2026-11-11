#include "TextEditorScene.h"

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

    if(context.input.wasKeyDAS(Key::Up)){
        if(cursRow > 0){
            cursRow--;
            if(cursRow < scrollOffset){
                scrollOffset = cursRow;
            }
            if(cursCol > static_cast<int>(lines[cursRow].size())){
                cursCol = lines[cursRow].size();
            }
        }
    }
    if(context.input.wasKeyDAS(Key::Down)){
        if(cursRow < static_cast<int>(lines.size() - 1)){
             cursRow++;
             if(cursRow >= scrollOffset + maxVisibleLines){
                scrollOffset = cursRow - maxVisibleLines + 1;
             }
             if(cursCol > static_cast<int>(lines[cursRow].size())){
                cursCol = lines[cursRow].size();
            }
        };
    }
    if(context.input.wasKeyDAS(Key::Left)){
        if(cursCol > 0) cursCol--;
    }
    if(context.input.wasKeyDAS(Key::Right)){
        if(cursCol < static_cast<int>(lines[cursRow].size())) cursCol++;
    }

    if(context.input.wasKeyDAS(Key::Return)){
        std::string currentLine = lines[cursRow];
        std::string remainder = currentLine.substr(cursCol);

        lines[cursRow] = currentLine.substr(0, cursCol);
        lines.insert(lines.begin() + cursRow + 1, remainder);

        cursRow++;
        cursCol = 0;
        isDirty = true;
    }

    if(context.input.wasKeyDAS(Key::Backspace)){
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

    if(context.input.isKeyDown(Key::LCTRL) && context.input.wasKeyPressed(Key::O)){
        std::string path = openNativeOpenDialog();
        if(!path.empty()){
            openFile(path);
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
    size_t endLine = std::min(lines.size(), static_cast<size_t>(scrollOffset + maxVisibleLines));

    renderer.setColor(ColorCodes::WHITE);
    for(size_t i = scrollOffset; i < endLine; i++){
        if(lines[i].empty()) continue;
        if(i >= lineTexture.size()) break;

        float visualY = startY + ((i - scrollOffset) * lineSpacing);

        Rect line = {
            startX,
            visualY,
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
            startY + ((cursRow - scrollOffset)*lineSpacing),
            3.0f,
            static_cast<float>(height),
        };
        renderer.drawRect(cursRect, true);
    };
}

void TextEditorScene::saveFile(const std::string& filename){
    std::filesystem::path pathObj(filename);
    std::string ext = pathObj.extension().string();

    if(ext == ".txt" || ext == ".md" || ext.empty()){
        std::ofstream outFile(filename);
        if(!outFile.is_open()) return;
        for(size_t i = 0; i < lines.size(); i++){
            outFile << lines[i];
        }
    }else if(ext == ".trk"){
        std::ofstream outFile(filename, std::ios::binary);
        if(!outFile.is_open()) return;

        const char* str = "T1RK";
        outFile.write(str, 4);

        int lineIdx = 0;

        auto safeLine = [&]() -> std::string{
            if(lineIdx < lines.size()) return lines[lineIdx++];
            return "0";
        };

        // 1) bpm
        float bpmVal;
        std::stringstream(safeLine()) >> bpmVal;
        outFile.write(reinterpret_cast<const char*>(&bpmVal), sizeof(float));
        // 2) num sections
        int tempSection;
        std::stringstream(safeLine()) >> tempSection;
        uint8_t numSection = static_cast<uint8_t>(tempSection);
        outFile.write(reinterpret_cast<const char*>(&numSection), sizeof(uint8_t));
        // for each num section
        for(uint8_t i = 0; i < numSection; i++){
            // 3) num tracks
            int tempTracks;
            std::stringstream(safeLine()) >> tempTracks;
            uint8_t numTracks = static_cast<uint8_t>(tempTracks);
            outFile.write(reinterpret_cast<const char*>(&numTracks), sizeof(uint8_t));
            // for each num track
            for(uint8_t j = 0; j < numTracks; j++){
                // 4) volume
                float volume;
                std::stringstream(safeLine()) >> volume;
                outFile.write(reinterpret_cast<const char*>(&volume), sizeof(float));
                // 5) num notes
                int tempNotes;
                std::stringstream(safeLine()) >> tempNotes;
                uint32_t numNotes = static_cast<uint32_t>(tempNotes);
                outFile.write(reinterpret_cast<const char*>(&numNotes), sizeof(uint32_t));
                // for each note
                if(numNotes > 0){
                    std::stringstream noteParser(safeLine());
                    for(uint32_t k = 0; k < numNotes; k++){
                        if(k > 0 && k % 11 == 0){
                            noteParser.str(""); noteParser.clear();
                            noteParser.str(safeLine());
                        }
                        float freq, duration;
                        char junk;
                        noteParser >> freq >> junk >> duration;
                        if((k+1) % 11 != 0 && k < numNotes - 1){
                            noteParser >> junk;
                        }
                        outFile.write(reinterpret_cast<const char*>(&freq), sizeof(float));
                        outFile.write(reinterpret_cast<const char*>(&duration), sizeof(float));
                    }
                }
            }
        }
        // 7) num arr
        int tempArr;
        std::stringstream(safeLine()) >> tempArr;
        uint8_t numArrangment = static_cast<uint8_t>(tempArr);
        outFile.write(reinterpret_cast<const char*>(&numArrangment), sizeof(uint8_t));
        // for each arrangment
        if(numArrangment > 0){
            std::stringstream arrParser(safeLine());
            for(uint8_t a = 0; a < numArrangment; a++){
                int tempArr;
                char junk;
                arrParser >> tempArr;

                uint8_t arrange = static_cast<uint8_t>(tempArr);
                outFile.write(reinterpret_cast<const char*>(&arrange), sizeof(uint8_t));

                if (a < numArrangment-1){
                    arrParser >> junk;
                }
            }
        }
        outFile.close();
    }else if(ext == ".tsm"){
        std::ofstream outFile(filename, std::ios::binary);
        if(!outFile.is_open()) return;

        const char* str = "TSM8";
        outFile.write(str, 4);

        int lineIdx = 0;

        auto safeLine = [&]() -> std::string{
            if(lineIdx < lines.size()) return lines[lineIdx++];
            return "0";
        };

        int tempPix = 0;
        std::stringstream(safeLine()) >> tempPix;
        uint8_t numPix = static_cast<uint8_t>(tempPix);
        outFile.write(reinterpret_cast<const char*>(&numPix), sizeof(uint8_t));
        for(uint8_t i = 0; i < numPix*2; i++){
            std::stringstream rowParser(safeLine());
            for(uint8_t j = 0; j < numPix; j++){
                int tmp;
                rowParser >> tmp;
                uint8_t val = static_cast<uint8_t>(tmp);
                outFile.write(reinterpret_cast<const char*>(&val), sizeof(uint8_t));
            }
        }
        outFile.close();
    }
}

void TextEditorScene::openFile(const std::string& filename){
    lines.clear();

    if(filename.empty()){
        lines.push_back("");
        isDirty = true;
        return;
    }

    std::filesystem::path pathObj(filename);
    std::string ext = pathObj.extension().string();

    if(ext == ".txt" || ext == ".md" || ext.empty()){
        std::ifstream inFile(filename);
        if(inFile.is_open()){
            std::string line;
            while(std::getline(inFile, line)){
                lines.push_back(line);
            }
            inFile.close();
        }
    }else if(ext == ".trk"){
        std::ifstream inFile(filename, std::ios::binary);
        if(inFile.is_open()){
            char sign[4];
            inFile.read(sign, 4);
            if(strncmp(sign, "T1RK", 4)) return;
            // 1) bpm
            float bpmVal = 0.0f;
            inFile.read(reinterpret_cast<char*>(&bpmVal), sizeof(float));
            std::ostringstream ss;
            ss << bpmVal;
            if(inFile) lines.push_back(ss.str());
            // 2) num sections
            uint8_t numSection = 0;
            inFile.read(reinterpret_cast<char*>(&numSection), sizeof(uint8_t));
            ss.str("");
            ss.clear();
            ss << static_cast<int>(numSection);
            if(inFile) lines.push_back(ss.str());
            // for each num section
            for(uint8_t i = 0; i < numSection; i++){
                // 3) num tracks
                uint8_t numTracks = 0;
                inFile.read(reinterpret_cast<char*>(&numTracks), sizeof(uint8_t));
                ss.str("");
                ss.clear();
                ss << static_cast<int>(numTracks);
                if(inFile) lines.push_back(ss.str());
                // for each num track
                for(uint8_t j = 0; j < numTracks; j++){
                    // 4) volume
                    float volume = 0.0f;
                    inFile.read(reinterpret_cast<char*>(&volume), sizeof(float));
                    ss.str("");
                    ss.clear();
                    ss << volume;
                    if(inFile) lines.push_back("    " + ss.str());
                    // 5) num notes
                    uint32_t numNotes = 0;
                    inFile.read(reinterpret_cast<char*>(&numNotes), sizeof(uint32_t));
                    ss.str("");
                    ss.clear();
                    ss << static_cast<int>(numNotes);
                    if(inFile) lines.push_back("    " + ss.str());
                    // for each note
                    ss.str(""); ss.clear();
                    for(uint8_t k = 0; k < numNotes; k++){
                        float freq = 0.0f;
                        float duration = 0.0f;
                        inFile.read(reinterpret_cast<char*>(&freq), sizeof(float));
                        inFile.read(reinterpret_cast<char*>(&duration), sizeof(float));
                        ss << static_cast<float>(freq) << ", " << static_cast<float>(duration);
                        if((k+1) % 11 == 0){
                            if(inFile) lines.push_back(ss.str());
                            ss.str(""); ss.clear();
                        }else{
                            ss << "; ";
                        }
                    }
                    if(inFile && !ss.str().empty()) lines.push_back(ss.str());
                }
            }
            // 7) num arr
            uint8_t numArrangements = 0;
            inFile.read(reinterpret_cast<char*>(&numArrangements), sizeof(uint8_t));
            ss.str("");
            ss.clear();
            ss << static_cast<int>(numArrangements);
            if(inFile) lines.push_back(ss.str());
            // for each arrangement
            if(numArrangements > 0){
                ss.str(""); ss.clear();
                for(uint8_t a = 0; a < numArrangements; a++){
                    uint8_t arrVal = 0;
                    inFile.read(reinterpret_cast<char*>(&arrVal), sizeof(uint8_t));
                    ss << static_cast<int>(arrVal);
                    if (a < numArrangements - 1) ss << ", ";
                }
                if(inFile) lines.push_back(ss.str());
            }

            inFile.close();
        }
    }else if(ext == ".tsm"){
        std::ifstream inFile(filename, std::ios::binary);
        if(inFile.is_open()){
            char sign[4];
            inFile.read(sign, 4);
            if(!strncmp(sign, "TSM8", 4)){
                std::ostringstream ss;
                uint8_t numPix = 0;
                inFile.read(reinterpret_cast<char*>(&numPix), sizeof(uint8_t));
                ss << static_cast<int>(numPix);
                lines.push_back(ss.str());
                ss.str(""); ss.clear();
                if(numPix > 0){
                    for(uint8_t i = 0; i < numPix*2; i++){
                        for(uint8_t j = 0; j < numPix; j++){
                            uint8_t val;
                            inFile.read(reinterpret_cast<char*>(&val), sizeof(uint8_t));
                            ss << static_cast<int>(val) << " ";
                        }
                        lines.push_back(ss.str());
                        ss.str(""); ss.clear();
                    }
                }
            }
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
