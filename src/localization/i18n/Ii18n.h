#pragma once

#include "src/helpers/_const.hpp"

class ITranslator {
    public:
        virtual std::string Menu_File() const = 0;
        virtual std::string Menu_Tools() const = 0;
        virtual std::string Menu_Patchnotes(const std::string &version) = 0;
        virtual std::string Menu_Help() const = 0;
        virtual std::string Menu_Options() const = 0;
        virtual std::string Menu_About() const = 0;
        virtual std::string Menu_OpenLog() const = 0;
        virtual std::string Menu_OpenLatestLog() const = 0;
        virtual std::string Menu_Quit() const = 0;
        virtual std::string Menu_CheckForUpgrades() const = 0;
        virtual std::string Alert_UpdateAvailable_Title() const = 0;
        virtual std::string Alert_UpdateAvailable_Text() const = 0;
        virtual std::string SearchingForUpdates() const = 0;
        virtual std::string Menu_OpenDataFolder(const std::string &folder) = 0;
};
