#pragma once

#include "AtomsHandler.h"
#include "src/shared/command/RPZActions.hpp"
#include "src/shared/models/Payloads.h"

#include <QAction>
#include <QMenu>

class AtomsContextualMenuHandler {

    public:
        AtomsContextualMenuHandler(AtomsHandler* hintsToContextualize) : _hints(hintsToContextualize) {}

        void removeSelectedAtoms() {
            auto selectedIds = this->_selectedAtomIds();
            this->_hints->handleAlterationRequest(RemovedPayload(selectedIds));
        }

        void moveSelectedAtomsToLayer(int targetLayer) {
            auto selectedIds = this->_selectedAtomIds();
            this->_hints->handleAlterationRequest(LayerChangedPayload(selectedIds, targetLayer));
        }

        void alterSelectedAtomsVisibility(bool isHidden) {
            auto selectedIds = this->_selectedAtomIds();
            this->_hints->handleAlterationRequest(VisibilityPayload(selectedIds, isHidden));
        }

        void alterSelectedAtomsAvailability(bool isLocked) {
            auto selectedIds = this->_selectedAtomIds();
            this->_hints->handleAlterationRequest(LockingPayload(selectedIds, isLocked));
        }

        void invokeMenu(int topMostLayer, int bottomMostLayer, const QPoint &whereToDisplay) {
            
            //display menu
            QMenu menu;
            menu.addActions(this->_genLayerActions(topMostLayer, bottomMostLayer));
            menu.addSeparator();
            menu.addActions(this->_genVisibilityActions());
            menu.addSeparator();
            menu.addActions(this->_genAvailabilityActions());
            menu.addSeparator();
            menu.addAction(this->_genRemoveAction());

            menu.exec(whereToDisplay);
        }


    protected:
        virtual QVector<snowflake_uid> _selectedAtomIds() = 0;

    private:
        AtomsHandler* _hints = nullptr;  

        QAction* _genRemoveAction() {
            auto del = RPZActions::remove();
            QObject::connect(
                del, &QAction::triggered,
                [=]() {this->removeSelectedAtoms();}
            );
            return del;
        }

        QList<QAction*> _genLayerActions(int riseLayoutTarget, int lowerLayoutTarget) {

            //list of layer actions to bind to menu
            QList<QAction*> layerActions;
            
                //rise...
                auto riseAction = RPZActions::raiseAtom(riseLayoutTarget);
                QObject::connect(
                    riseAction, &QAction::triggered,
                    [=]() {this->moveSelectedAtomsToLayer(riseLayoutTarget);}
                );
                layerActions.append(riseAction);

                //lower...
                auto lowerAction = RPZActions::lowerAtom(lowerLayoutTarget);
                QObject::connect(
                    lowerAction, &QAction::triggered,
                    [=]() {this->moveSelectedAtomsToLayer(lowerLayoutTarget);}
                );
                layerActions.append(lowerAction);
            
            return layerActions;
        }

        QList<QAction*> _genVisibilityActions() {
            QList<QAction*> out;

            auto showAction = RPZActions::showAtom();
            QObject::connect(
                showAction, &QAction::triggered,
                [=]() {this->alterSelectedAtomsVisibility(false);}
            );
            out.append(showAction);

            auto hideAction = RPZActions::hideAtom();
            QObject::connect(
                hideAction, &QAction::triggered,
                [=]() {this->alterSelectedAtomsVisibility(true);}
            );
            out.append(hideAction);

            return out;
        }

        QList<QAction*> _genAvailabilityActions() {
            QList<QAction*> out;


            auto lockAction = RPZActions::lockAtom();
            QObject::connect(
                lockAction, &QAction::triggered,
                [=]() {this->alterSelectedAtomsAvailability(true);}
            );
            out.append(lockAction);

            auto unlockAction = RPZActions::unlockAtom();
            QObject::connect(
                unlockAction, &QAction::triggered,
                [=]() {this->alterSelectedAtomsAvailability(false);}
            );
            out.append(unlockAction);

            return out;
        }
};