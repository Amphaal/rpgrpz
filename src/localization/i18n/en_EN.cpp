#include "Ii18n.h"

using namespace std;

class Translator_EN : public ITranslator {
    string Menu_File() { return "File"; }
    string Menu_Options() { return "Options"; }
    string Menu_Quit() { return "Quit"; };
    string Menu_CheckForUpgrades() { return "Check for upgrades"; };
    string Alert_UpdateAvailable_Title() { return "Update Available"; };
    string Alert_UpdateAvailable_Text() { return "An update is available for " + (std::string)APP_NAME + ". Would you like to install it now ?"; };
    string SearchingForUpdates() { return "Searching for updates...";};
};