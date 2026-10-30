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
    ofn.lpstrFilter = "Text Files (*.txt*)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    if(GetSaveFileNameA(&ofn) == TRUE){
        return std::string(ofn.lpstrFile);
    }
    return "";
}
