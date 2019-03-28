#include "Ii18n.h"

using namespace std;

class Translator_FR : public ITranslator {
    std::string Menu_File() { return "Fichier"; }
    std::string Menu_Options() { return "Options"; }
    std::string Menu_Quit() { return "Quitter"; };
    std::string Menu_About() { return "À Propos"; }
    std::string Menu_OpenLog() { return "Ouvrir le fichier de log complet"; };
    std::string Menu_OpenLatestLog() { return "Ouvrir le fichier de log de la session"; };
    std::string Menu_CheckForUpgrades() { return "Vérifier les mises à jour"; };
    std::string Alert_UpdateAvailable_Title() { return "Mise à jour disponible"; };
    std::string Alert_UpdateAvailable_Text() { return "Une mise à jour a été trouvé pour " + (std::string)APP_NAME + ". Voulez-vous l'installer maintenant ?"; };
    std::string SearchingForUpdates() { return "Recherche des mises à jour...";};
    std::string Menu_OpenDataFolder(std::string folder) { return "Ouvrir le dossier de données (" + folder + ") "; };
};