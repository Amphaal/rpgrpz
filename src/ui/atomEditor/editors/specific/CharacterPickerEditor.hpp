#pragma once

#include <QComboBox>

#include "src/ui/atomEditor/_base/AtomSubEditor.h"
#include "src/ui/_others/ConnectivityObserver.h"
#include "src/shared/hints/HintThread.hpp"

class CharacterPickerEditor : public AtomSubEditor, public ConnectivityObserver {

    Q_OBJECT

    protected:
        void connectingToServer() override {
            
            QObject::connect(
                this->_rpzClient, &RPZClient::userDataChanged,
                this, &CharacterPickerEditor::_mayUpdateCharacterName
            );

            QObject::connect(
                this->_rpzClient, &RPZClient::charactersCountChanged,
                [=]() {
                    this->_availableCharacters = this->_rpzClient->sessionCharacters();
                    this->_updateComboFromAvailableCharacters();
                }
            );

        }

        void connectionClosed(bool hasInitialMapLoaded) override {
            this->_availableCharacters.clear();
            this->_updateComboFromAvailableCharacters();
        }

    private:
        QList<RPZCharacter> _availableCharacters;
        QPair<RPZCharacter::Id, QString> _defaultCharacter;

        void _updateComboFromAvailableCharacters() {

            auto fillWith = HintThread::hint()->findUnboundCharacters(this->_availableCharacters);
            
            //clear combo
            QSignalBlocker b(this->_combo);
            this->_combo->clear();

            //if combo is disabled, just skip
            if(!this->_combo->isEnabled()) {
                this->_combo->addItem(tr("[Cannot configure...]"), 0);
                return;
            } 
            
            //add default empty option
            else {  
                this->_combo->addItem(tr("[No character associated]"), 0);
            }

            //fill
            for(const auto &character : fillWith) {
                this->_combo->addItem(QIcon(":/icons/app/connectivity/cloak.png"), character.toString(), character.id());
            }

            //find default
            auto indexFound = this->_combo->findData(this->_defaultCharacter.first);
            
            //found !
            if(indexFound > -1) {
                this->_combo->setCurrentIndex(indexFound); //define as default
            }

            //not found...
            else {

                this->_combo->addItem(
                    QIcon(":/icons/app/connectivity/cloak.png"), 
                    this->_defaultCharacter.second, 
                    this->_defaultCharacter.first
                );

                //define as default
                this->_combo->setCurrentIndex(this->_combo->count() - 1);

            }

        }   
        
        void _mayUpdateCharacterName(const RPZUser &changed) {
            
            //prevent update if not player
            if(changed.role() != RPZUser::Role::Player) return;
            
            //search for character in combo
            auto character = changed.character();
            auto index = this->_combo->findData(character.id());
            if(index == -1) return;

            //update name
            this->_combo->setItemText(index, character.toString());

        }

        QComboBox* _combo = nullptr;
        void _fillComboDefaultValues(const RPZAtom::Updates &defaultValues, const AtomSubEditor::LoadingContext &context) {

            //define combo state
            auto allows = context.mode == AtomSubEditor::EditMode::Selection && context.numberOfItems == 1;
            this->_combo->setEnabled(allows);

            //check for a bound characterId
            auto characterId = defaultValues.value(this->_params.first()).toULongLong();
            
            //define default character
            if(characterId) {
                auto name = defaultValues.value(this->_params.last()).toString();
                this->_defaultCharacter = { characterId, name };
            } 
            
            //reset default character
            else {
                this->_defaultCharacter = {};
            }

            //fill combo 
            this->_updateComboFromAvailableCharacters();

        }

        void _onComboValueChanged(int index) {
            auto characterId = this->_combo->currentData().toULongLong();
            auto name = characterId ? this->_combo->currentText() : "";
            
            emit valueConfirmedForPayload({
                { this->_params.first(), characterId },
                { this->_params.last(), name }
            });
        }

    public:
        CharacterPickerEditor() : AtomSubEditor({RPZAtom::Parameter::CharacterId, RPZAtom::Parameter::CharacterName}, false) { 

            this->setVisible(false);

            this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
            
            this->_combo = new QComboBox;
            
            QObject::connect(
                this->_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &CharacterPickerEditor::_onComboValueChanged
            );

            this->layout()->addWidget(this->_combo);

        };

        void loadTemplate(const RPZAtom::Updates &defaultValues, const AtomSubEditor::LoadingContext &context) override {
            AtomSubEditor::loadTemplate(defaultValues, context);
            this->_fillComboDefaultValues(defaultValues, context);
        }

};