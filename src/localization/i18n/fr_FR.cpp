#include "Ii18n.h"

using namespace std;

class Translator_FR : public ITranslator {
    std::string Menu_File() const { return "Fichier"; }
    std::string Menu_Tools() const { return "Outils"; }
    std::string Menu_OpenMaintenanceTool() const { return "Ouvrir l'outil de maintenance"; };
    std::string Menu_Patchnotes(const std::string &version) { return version + " - Notes de publication"; }
    std::string Menu_Help() const { return "Aide"; }
    std::string Menu_Options() const { return "Options"; }
    std::string Menu_Quit() const { return "Quitter"; };
    std::string Menu_About() const { return "À Propos"; }
    std::string Menu_OpenLog() const { return "Ouvrir le fichier de log complet"; };
    std::string Menu_OpenLatestLog() const { return "Ouvrir le fichier de log de la session"; };
    std::string Menu_CheckForUpgrades() const { return "Vérifier les mises à jour..."; };
    std::string Alert_UpdateAvailable_Title() const { return "Mise à jour disponible"; };
    std::string Alert_UpdateAvailable_Text() const { return "Une mise à jour a été trouvé pour " + (std::string)APP_NAME + ". Voulez-vous l'installer maintenant ?"; };
    std::string SearchingForUpdates() const { return "Recherche des mises à jour...";};
    std::string Menu_OpenDataFolder(const std::string &folder) { return "Ouvrir le dossier de données (" + folder + ") "; };
};