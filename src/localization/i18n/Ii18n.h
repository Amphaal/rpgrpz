#pragma once

#include "src/helpers/_const.cpp"

class ITranslator {
    public:
        virtual std::string Menu_File() = 0;
        virtual std::string Menu_Options() = 0;
        virtual std::string Menu_About() = 0;
        virtual std::string Menu_OpenLog() = 0;
        virtual std::string Menu_OpenLatestLog() = 0;
        virtual std::string Menu_Quit() = 0;
        virtual std::string Menu_CheckForUpgrades() = 0;
        virtual std::string Alert_UpdateAvailable_Title() = 0;
        virtual std::string Alert_UpdateAvailable_Text() = 0;
        virtual std::string SearchingForUpdates() = 0;
        virtual std::string Menu_OpenDataFolder(std::string folder) = 0;
};
