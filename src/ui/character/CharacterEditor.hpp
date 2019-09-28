#pragma once

#include <QWidget>
#include "CharacterSheet.hpp"
#include "src/shared/database/CharactersDatabase.hpp"

#include "components/CharacterPicker.hpp"

class CharacterEditor : public QWidget {
    public:
        CharacterEditor() : _characterPicker(new CharacterPicker), _sheet(new CharacterSheet) {
                        
            //layout
            auto l = new QVBoxLayout;
            this->setLayout(l);
            l->setAlignment(Qt::AlignTop);
            l->addWidget(this->_characterPicker);
            l->addWidget(this->_sheet);
            
            QObject::connect(
                this->_characterPicker, &CharacterPicker::selectionChanged,
                this->_sheet, &CharacterSheet::loadCharacter
            );

            this->_characterPicker->loadCharacters();

        }
    
    private:
        CharacterPicker* _characterPicker = nullptr;
        CharacterSheet* _sheet = nullptr;
};