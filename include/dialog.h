#include <windows.h>
#include <commdlg.h>
#include <string>

inline std::string openNativeSaveDialog(){
    OPENFILENAMEA ofn;
    char szFile[260] = { 0 };

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "T1tan Record Keys (*.trk*)\0*.trk\0T1tan Sprite Map 8bit (*.tsm*)\0*.tsm\0Text Files (*.txt*)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    if(GetSaveFileNameA(&ofn) == TRUE){
        return std::string(ofn.lpstrFile);
    }
    return "";
}

inline std::string openNativeOpenDialog(){
    OPENFILENAMEA ofn;
    char szFile[260] = { 0 };

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "T1tan Record Keys (*.trk*)\0*.trk\0T1tan Sprite Map 8bit (*.tsm*)\0*.tsm\0Text Files (*.txt*)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    if(GetOpenFileNameA(&ofn) == TRUE){
        return std::string(ofn.lpstrFile);
    }
    return "";
}
