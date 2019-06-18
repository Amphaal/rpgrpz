#include "Ii18n.h"

using namespace std;

class Translator_FR : public ITranslator {
    QString Menu_File() const { return "Fichier"; }
    QString Menu_Tools() const { return "Outils"; }
    QString Menu_OpenMaintenanceTool() const { return "Ouvrir l'outil de maintenance"; };
    QString Menu_Patchnotes(const QString &version) { return version + " - Notes de publication"; }
    QString Menu_Help() const { return "Aide"; }
    QString Menu_Options() const { return "Options"; }
    QString Menu_Quit() const { return "Quitter"; };
    QString Menu_About() const { return "À Propos"; }
    QString Menu_OpenLog() const { return "Ouvrir le fichier de log complet"; };
    QString Menu_OpenLatestLog() const { return "Ouvrir le fichier de log de la session"; };
    QString Menu_CheckForUpgrades() const { return "Vérifier les mises à jour..."; };
    QString Alert_UpdateAvailable_Title() const { return "Mise à jour disponible"; };
    QString Alert_UpdateAvailable_Text() const { return "Une mise à jour a été trouvé pour " + QString(APP_NAME) + ". Voulez-vous l'installer maintenant ?"; };
    QString SearchingForUpdates() const { return "Recherche des mises à jour...";};
    QString Menu_OpenDataFolder(const QString &folder) { return "Ouvrir le dossier de données (" + folder + ") "; };
    QString Popup_MapDescriptor() { return "Carte RPGZ (*" +  AppContext::RPZ_MAP_FILE_EXT + ")"; };
};