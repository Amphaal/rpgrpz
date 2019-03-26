#include "Ii18n.h"

using namespace std;

class Translator_FR : public ITranslator {
    string Menu_File() { return "Fichier"; }
    string Menu_Options() { return "Options"; }
    string Menu_Quit() { return "Quitter"; };
    string Menu_CheckForUpgrades() { return "Vérifier les mises à jour"; };
    string Alert_UpdateAvailable_Title() { return "Mise à jour disponible"; };
    string Alert_UpdateAvailable_Text() { return "Une mise à jour a été trouvé pour " + (std::string)APP_NAME + ". Voulez-vous l'installer maintenant ?"; };
    string SearchingForUpdates() { return "Recherche des mises à jour...";};
};