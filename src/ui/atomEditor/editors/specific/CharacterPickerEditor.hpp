#pragma once

#include <QComboBox>

#include "src/ui/atomEditor/_base/AtomSubEditor.h"
#include "src/ui/_others/ConnectivityObserver.h"

class CharacterPickerEditor : public AtomSubEditor, public ConnectivityObserver {

    Q_OBJECT

    protected:
        void connectingToServer() override {
            
            QObject::connect(
                this->_rpzClient, &RPZClient::userDataChanged,
                this, &CharacterPickerEditor::_mayUpdateCharacterName
            );

            //TODO

        }

        void connectionClosed(bool hasInitialMapLoaded) override {
            QObject::connect(
                this->_rpzClient, &RPZClient::userDataChanged,
                this, &CharacterPickerEditor::_mayUpdateCharacterName
            );
        }

    private:    
        void _mayUpdateCharacterName(const RPZUser &changed) {
            
            if(changed.role() != RPZUser::Role::Player) return;
            
            auto character = changed.character();
            auto index = this->_combo->findData(character.id());
            if(index == -1) return;

            this->_combo->setItemText(index, character.toString());

        }

        QComboBox* _combo = nullptr;
        void _fillCombo(const RPZAtom::Updates &defaultValues, const AtomSubEditor::LoadingContext &context) {
            
            //clear combo
            QSignalBlocker b(this->_combo);
            this->_combo->clear();

            //check if we can enable combo
            auto allows = context.mode == AtomSubEditor::EditMode::Selection && context.numberOfItems == 1;
            this->_combo->setEnabled(allows);
            if(!allows) {
                this->_combo->addItem(tr("[Cannot configure...]"), 0);
                return;
            }

            //add default
            this->_combo->addItem(tr("[No character associated]"), 0); //no character, default

            //check for characterId
            auto characterId = defaultValues.value(this->_params.first()).toULongLong();
            if(characterId) {
                auto name = defaultValues.value(this->_params.last()).toString();
                this->_combo->addItem(QIcon(":/icons/app/connectivity/cloak.png"), name, characterId);
                this->_combo->setCurrentIndex(this->_combo->count() - 1);
            }

        }

    public:
        CharacterPickerEditor() : AtomSubEditor({RPZAtom::Parameter::CharacterId, RPZAtom::Parameter::CharacterName}, false) { 

            this->setVisible(false);

            this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
            
            this->_combo = new QComboBox;
            
            QObject::connect(
                this->_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                [&](int currentIndex) {
                    
                    auto characterId = this->_combo->currentData().toULongLong();
                    auto name = characterId ? this->_combo->currentText() : "";
                    
                    emit valueConfirmedForPayload({
                        { this->_params.first(), characterId },
                        { this->_params.last(), name }
                    });
                    
                }
            );

            this->layout()->addWidget(this->_combo);

        };

        void loadTemplate(const RPZAtom::Updates &defaultValues, const AtomSubEditor::LoadingContext &context) override {
            
            AtomSubEditor::loadTemplate(defaultValues, context);

            this->_fillCombo(defaultValues, context);

            //TODO adapt with current players on game
            
        }

};