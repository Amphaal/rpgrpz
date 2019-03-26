#pragma once
#include <string>

#include "src/helpers/_const.cpp"

using namespace std;

class ITranslator {
    public:
        virtual string Menu_File() = 0;
        virtual string Menu_Options() = 0;
        virtual string Menu_Quit() = 0;
        virtual string Menu_CheckForUpgrades() = 0;
        virtual string Alert_UpdateAvailable_Title() = 0;
        virtual string Alert_UpdateAvailable_Text() = 0;
        virtual string SearchingForUpdates() = 0;
};
