#include "Ii18n.h"

using namespace std;

class Translator_EN : public ITranslator {
    std::string Menu_File() { return "File"; }
    std::string Menu_Tools() { return "Tools"; }
    std::string Menu_Patchnotes(std::string version) { return version + " - Patch Notes"; }
    std::string Menu_Help() { return "Help"; }
    std::string Menu_Options() { return "Options"; }
    std::string Menu_Quit() { return "Quit"; };
    std::string Menu_About() { return "About"; }
    std::string Menu_OpenLog() { return "Open full log file"; };
    std::string Menu_OpenLatestLog() { return "Open the session log file"; };
    std::string Menu_CheckForUpgrades() { return "Check for upgrades..."; };
    std::string Alert_UpdateAvailable_Title() { return "Update Available"; };
    std::string Alert_UpdateAvailable_Text() { return "An update is available for " + (std::string)APP_NAME + ". Would you like to install it now ?"; };
    std::string SearchingForUpdates() { return "Searching for updates...";};
    std::string Menu_OpenDataFolder(std::string folder) { return "Open Data Folder (" + folder + ") "; };
};