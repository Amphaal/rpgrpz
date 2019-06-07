#pragma once

#include "src/localization/i18n.hpp"

#include <QAction>

class RPZActions {
    public:
        static QAction* createFolder() {
            return new QAction("Créer un dossier");
        }

        static QAction* remove() {
            return new QAction("Supprimer");
        }

        static QAction* raiseAtom(int targetLayer) {
            return new QAction(
                QString("Remonter (Calque %1)")
                    .arg(QString::number(targetLayer))
            );
        }

        static QAction* lowerAtom(int targetLayer) {
            return new QAction(
                QString("Descendre (Calque %1)")
                    .arg(QString::number(targetLayer))
            );
        }

        static QAction* showAtom() {
            return new QAction("Montrer");
        }

        static QAction* hideAtom() {
            return new QAction("Cacher");
        }

        static QAction* selectionTool() {
            auto selection = new QAction;
            selection->setIcon(QIcon(":/icons/app/tools/cursor.png"));
            selection->setIconText("Outil de sélection");
            selection->setCheckable(true);
            return selection;
        }

        static QAction* writeTool() {
            auto text = new QAction;
            text->setIcon(QIcon(":/icons/app/tools/text.png"));
            text->setIconText("Ecrire");
            text->setCheckable(true);
            return text;
        }

        static QAction* drawTool() {
            auto draw = new QAction;
            draw->setIcon(QIcon(":/icons/app/tools/pen.png"));
            draw->setIconText("Dessiner");
            draw->setCheckable(true);
            return draw;
        }

        static QAction* resetView() {
            auto reset = new QAction;
            reset->setIcon(QIcon(":/icons/app/tools/reset.png"));
            reset->setIconText("Réinitialiser la vue");
            return reset;
        }

        static QAction* lockAtom() {
            return new QAction("Verouiller");
        }

        static QAction* unlockAtom() {
            return new QAction("Déverouiller");
        }

        static QAction* quit() {
            auto quitAction = new QAction(I18n::tr()->Menu_Quit());
            quitAction->setShortcut(QKeySequence::Close);
            return quitAction;
        }

        static QAction* patchnote() {
           return new QAction(I18n::tr()->Menu_Patchnotes(APP_FULL_DENOM));
        }

        static QAction* checkUpdates() {
            return new QAction(I18n::tr()->Menu_CheckForUpgrades());
        }

        static QAction* openInternalDataFolder() {
            return new QAction(I18n::tr()->Menu_OpenDataFolder(AppContext::getAppDataLocation()));
        }

        static QAction* openLatestLog() {
            return new QAction(I18n::tr()->Menu_OpenLatestLog());
        }

        static QAction* openFullLog() {
            return new QAction(I18n::tr()->Menu_OpenLog());
        }

        static QAction* openMaintenanceTool() {
            return new QAction(I18n::tr()->Menu_OpenMaintenanceTool());
        }

        static QAction* loadMap() {
            auto load = new QAction("Charger une carte");
            load->setShortcut(QKeySequence::Open);
            return load;
        }

        static QAction* saveMap() {
            auto save = new QAction("Sauvegarder la carte");
            save->setShortcut(QKeySequence::Save);
            return save;
        }

        static QAction* saveAsMap() {
            auto saveAs = new QAction("Enregistrer la carte sous...");
            saveAs->setShortcut(QKeySequence::SaveAs);
            return saveAs;
        }
};