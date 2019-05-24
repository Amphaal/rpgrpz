#pragma once

#include "src/helpers/_appContext.h"

class ITranslator {
    public:
        virtual QString Menu_File() const = 0;
        virtual QString Menu_OpenMaintenanceTool() const = 0;
        virtual QString Menu_Tools() const = 0;
        virtual QString Menu_Patchnotes(const QString &version) = 0;
        virtual QString Menu_Help() const = 0;
        virtual QString Menu_Options() const = 0;
        virtual QString Menu_About() const = 0;
        virtual QString Menu_OpenLog() const = 0;
        virtual QString Menu_OpenLatestLog() const = 0;
        virtual QString Menu_Quit() const = 0;
        virtual QString Menu_CheckForUpgrades() const = 0;
        virtual QString Alert_UpdateAvailable_Title() const = 0;
        virtual QString Alert_UpdateAvailable_Text() const = 0;
        virtual QString SearchingForUpdates() const = 0;
        virtual QString Menu_OpenDataFolder(const QString &folder) = 0;
        virtual QString Popup_MapDescriptor() = 0;
};
