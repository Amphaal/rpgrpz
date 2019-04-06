#include "Ii18n.h"

using namespace std;

class Translator_EN : public ITranslator {
    std::string Menu_File() const { return "File"; }
    std::string Menu_Tools() const { return "Tools"; }
    std::string Menu_Patchnotes(const std::string &version) { return version + " - Patch Notes"; }
    std::string Menu_Help() const { return "Help"; }
    std::string Menu_Options() const { return "Options"; }
    std::string Menu_Quit() const { return "Quit"; };
    std::string Menu_About() const { return "About"; }
    std::string Menu_OpenLog() const { return "Open full log file"; };
    std::string Menu_OpenLatestLog() const { return "Open the session log file"; };
    std::string Menu_CheckForUpgrades() const { return "Check for upgrades..."; };
    std::string Alert_UpdateAvailable_Title() const { return "Update Available"; };
    std::string Alert_UpdateAvailable_Text() const { return "An update is available for " + (std::string)APP_NAME + ". Would you like to install it now ?"; };
    std::string SearchingForUpdates() const { return "Searching for updates...";};
    std::string Menu_OpenDataFolder(const std::string &folder) { return "Open Data Folder (" + folder + ") "; };
};