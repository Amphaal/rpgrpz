#include "Ii18n.h"

class Translator_EN : public ITranslator {
    QString Menu_File() const { return "File"; }
    QString Menu_Tools() const { return "Tools"; }
    QString Menu_OpenMaintenanceTool() const { return "Open the maintenance tool"; };
    QString Menu_Patchnotes(const QString &version) { return version + " - Patch Notes"; }
    QString Menu_Help() const { return "Help"; }
    QString Menu_Options() const { return "Options"; }
    QString Menu_Quit() const { return "Quit"; };
    QString Menu_About() const { return "About"; }
    QString Menu_OpenLog() const { return "Open full log file"; };
    QString Menu_OpenLatestLog() const { return "Open the session log file"; };
    QString Menu_CheckForUpgrades() const { return "Check for upgrades..."; };
    QString Alert_UpdateAvailable_Title() const { return "Update Available"; };
    QString Alert_UpdateAvailable_Text() const { return "An update is available for " + QString(APP_NAME) + ". Would you like to install it now ?"; };
    QString SearchingForUpdates() const { return "Searching for updates...";};
    QString Menu_OpenDataFolder(const QString &folder) { return "Open Data Folder (" + folder + ") "; };
    QString Popup_MapDescriptor() { return "RPGZ Map (*" +  AppContext::RPZ_MAP_FILE_EXT + ")"; };
};