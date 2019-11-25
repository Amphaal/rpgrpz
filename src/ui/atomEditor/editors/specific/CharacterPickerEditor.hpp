#pragma once

#include <QComboBox>

#include "src/ui/atomEditor/_base/AtomSubEditor.h"
#include "src/ui/_others/ConnectivityObserver.h"
#include "src/shared/hints/HintThread.hpp"

class CharacterPickerEditor : public AtomSubEditor, public ConnectivityObserver {

    Q_OBJECT

    public:
        struct DefaultCharacterSelection {
            RPZCharacter::Id characterId = 0;
            QString characterName;
            QColor characterColor;
        };

        CharacterPickerEditor() : AtomSubEditor({RPZAtom::Parameter::CharacterId, RPZAtom::Parameter::CharacterName, RPZAtom::Parameter::DefaultPlayerColor}, false) { 

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

    protected:
        void connectingToServer() override {
            
            QObject::connect(
                this->_rpzClient, &RPZClient::userDataChanged,
                this, &CharacterPickerEditor::_mayUpdateCharacter
            );

            QObject::connect(
                this->_rpzClient, &RPZClient::charactersCountChanged,
                this, &CharacterPickerEditor::_onCharacterCountChange
            );

        }

        void connectionClosed(bool hasInitialMapLoaded) override {
            this->_availableCharacters.clear();
            this->_updateComboFromAvailableCharacters();
        }

    private:
        QList<RPZCharacter::UserBound> _availableCharacters;
        DefaultCharacterSelection _defaultCharacter;

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
            for(const auto &bond : fillWith) {
                
                const auto &character = bond.second;
                const auto &color = bond.first;

                DefaultCharacterSelection sel { character.id(), character.toString(), color };
                this->_addCharacterItem(sel);

            }

            //find default
            auto indexFound = this->_findItemIndexFromId(this->_defaultCharacter.characterId);
            
            //found !
            if(indexFound > -1) {
                this->_combo->setCurrentIndex(indexFound); //define as default
            }

            //not found...
            else {
                this->_addCharacterItem(this->_defaultCharacter, true);
            }

        }   
        
      

        int _findItemIndexFromId(const RPZCharacter::Id &id) {
            return this->_combo->findData(id);
        }

        void _addCharacterItem(const DefaultCharacterSelection &characterDescr, bool selected = false) {
                
            this->_combo->addItem(
                QIcon(":/icons/app/connectivity/cloak.png"), 
                characterDescr.characterName, 
                characterDescr.characterId
            );

            auto insertedAt = this->_combo->count() - 1;

            this->_combo->setItemData(
                insertedAt, 
                QVariant::fromValue(characterDescr), 
                257
            );

            if(selected) {
                this->_combo->setCurrentIndex(insertedAt);
            }

        }

        QComboBox* _combo = nullptr;
        void _fillComboDefaultValues(const RPZAtom::Updates &defaultValues, const AtomSubEditor::LoadingContext &context) {

            //define combo state
            auto allows = context.mode == AtomSubEditor::EditMode::Selection && context.numberOfItems == 1;
            this->setEnabled(allows);

            //check for a bound characterId
            auto characterId = defaultValues.value(RPZAtom::Parameter::CharacterId).toULongLong();
            
            //define default character
            if(characterId) {
                auto name = defaultValues.value(RPZAtom::Parameter::CharacterName).toString();
                auto color = defaultValues.value(RPZAtom::Parameter::DefaultPlayerColor).value<QColor>();
                this->_defaultCharacter = { characterId, name, color };
            } 
            
            //reset default character
            else {
                this->_defaultCharacter = {};
            }

            //fill combo 
            this->_updateComboFromAvailableCharacters();

        }

    private slots:
        void _onComboValueChanged(int index) {
            
            auto sel = this->_combo->currentData(257).value<DefaultCharacterSelection>();
            
            emit valueConfirmedForPayload({
                { RPZAtom::Parameter::CharacterId, sel.characterId },
                { RPZAtom::Parameter::CharacterName, sel.characterName },
                { RPZAtom::Parameter::DefaultPlayerColor, sel.characterColor }
            });

        }

        void _mayUpdateCharacter(const RPZUser &changed) {
            
            //prevent update if not player
            if(changed.role() != RPZUser::Role::Player) return;
            
            //search for character in combo
            auto character = changed.character();
            
            //find index of character
            auto index = this->_findItemIndexFromId(character.id());
            if(index == -1) return;

            //update name
            DefaultCharacterSelection sel { character.id(), character.toString(), changed.color() };
            
            this->_combo->setItemText(index, sel.characterName);
            this->_combo->setItemData(index, QVariant::fromValue(sel), 257);

        }

        void _onCharacterCountChange() {
            this->_availableCharacters = this->_rpzClient->sessionCharacters();
            this->_updateComboFromAvailableCharacters();
        }

   
};

Q_DECLARE_METATYPE(CharacterPickerEditor::DefaultCharacterSelection);