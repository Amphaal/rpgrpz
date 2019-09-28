#pragma once

#include <QWidget>
#include "CharacterSheet.hpp"
#include "src/shared/database/CharactersDatabase.hpp"

#include "components/CharacterPicker.hpp"

class CharacterEditor : public QWidget {
    public:
        ~CharacterEditor() {
            this->_saveCurrentCharacter();
        }

        CharacterEditor() : _characterPicker(new CharacterPicker), _sheet(new CharacterSheet), _saveCharacterBtn(new QPushButton) {

            //picker        
            QObject::connect(
                this->_characterPicker, &CharacterPicker::selectionChanged,
                this, &CharacterEditor::_onPickerSelectionChanged
            );
            QObject::connect(
                this->_characterPicker, &CharacterPicker::requestSave,
                this, &CharacterEditor::_onSaveRequest
            );

            //save character
            this->_saveCharacterBtn->setToolTip("Sauvegarder la fiche");
            this->_saveCharacterBtn->setIcon(QIcon(":/icons/app/other/save.png"));
            this->_saveCharacterBtn->setVisible(false);
            
            QObject::connect(
                this->_saveCharacterBtn, &QPushButton::pressed,
                this, &CharacterEditor::_onSaveButtonPressed
            );

            //layout
            auto l = new QVBoxLayout;
            this->setLayout(l);
            l->setAlignment(Qt::AlignTop);
            l->addWidget(this->_characterPicker);
            l->addWidget(this->_sheet);
            l->addWidget(this->_saveCharacterBtn, 0, Qt::AlignRight);

            //load
            this->_characterPicker->loadCharacters();

        }
    
    private:
        QPushButton* _saveCharacterBtn = nullptr;
        CharacterPicker* _characterPicker = nullptr;
        CharacterSheet* _sheet = nullptr;

        void _onSaveRequest(RPZCharacter* toSave) {
            this->_saveCharacter(toSave);
        }

        void _onSaveButtonPressed() {
            this->_saveCurrentCharacter();
            this->_characterPicker->updateBufferedItemString();       
        }

        void _saveCurrentCharacter() {
            auto charRef = this->_characterPicker->currentCharacter();
            this->_saveCharacter(charRef);
        }

        void _saveCharacter(RPZCharacter* toSave) {
            
            if(!toSave) return;

            this->_sheet->updateCharacter(*toSave);
            CharactersDatabase::get()->updateCharacter(*toSave);

        }

        void _onPickerSelectionChanged(const RPZCharacter* selected) {
            
            this->_saveCharacterBtn->setVisible(selected);
            auto characterTemplate = selected ? *selected : RPZCharacter();
            
            this->_sheet->loadCharacter(characterTemplate);

        }
};