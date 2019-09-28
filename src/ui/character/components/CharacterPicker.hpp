#pragma once

#include <QWidget>
#include <QComboBox>
#include <QHBoxLayout>
#include <QPushButton>

#include "src/shared/database/CharactersDatabase.hpp"

class CharacterPicker : public QWidget {

    Q_OBJECT

    signals:
        void selectionChanged(const RPZCharacter &selected);

    public:
        CharacterPicker() : _characterListCombo(new QComboBox), _deleteCharacterBtn(new QPushButton) {
            
            //add new character
            auto newCharacterBtn = new QPushButton;
            newCharacterBtn->setIcon(QIcon(":/icons/app/other/add.png"));
            newCharacterBtn->setToolTip("Créer une nouvelle fiche");
            newCharacterBtn->setMaximumWidth(25);
            QObject::connect(
                newCharacterBtn, &QPushButton::pressed,
                this, &CharacterPicker::_addButtonPressed
            );

            //delete character
            this->_deleteCharacterBtn->setToolTip("Supprimer la fiche");
            this->_deleteCharacterBtn->setMaximumWidth(25);
            this->_deleteCharacterBtn->setIcon(QIcon(":/icons/app/other/remove.png"));
            this->_deleteCharacterBtn->setEnabled(false);
            QObject::connect(
                this->_deleteCharacterBtn, &QPushButton::pressed,
                this, &CharacterPicker::_deleteButtonPressed
            );

            //on selected character changed
            QObject::connect(
                this->_characterListCombo, qOverload<int>(&QComboBox::currentIndexChanged),
                this, &CharacterPicker::_onSelectedIndexChanged
            );
            
            //layout
            auto clLayout = new QHBoxLayout;
            this->setLayout(clLayout);
            this->setContentsMargins(0,0,0,0);
            clLayout->addWidget(this->_characterListCombo, 1);
            clLayout->addWidget(this->_deleteCharacterBtn);
            clLayout->addWidget(newCharacterBtn);

        }

        void loadCharacters() {
            
            this->_characterListCombo->clear();
            auto characters = CharactersDatabase::get()->characters();
            
            //if no character in DB
            if(characters.isEmpty()) {
                this->_characterListCombo->addItem("Aucun personnage existant, créez en un !");
                this->_characterListCombo->setEnabled(false);
                return;
            }

            //add an item for each
            this->_characterListCombo->setEnabled(true);
            {
                QSignalBlocker b(this->_characterListCombo);
                for(auto &character : characters) {
                    this->_characterListCombo->addItem(character.toString(), character);
                }
            }

            //make sure to trigger event after refresh
            auto last = characters.count() - 1;
            auto ci = this->_characterListCombo->currentIndex();
            if(last != ci) {
                this->_characterListCombo->setCurrentIndex(last);
            } else {
                this->_onSelectedIndexChanged(ci);
            }
            
            
        }
    
    private:
        QComboBox* _characterListCombo = nullptr;
        QPushButton* _deleteCharacterBtn = nullptr;
        RPZCharacter _selectedCharacter;

        void _addButtonPressed() {
            CharactersDatabase::get()->addNewCharacter();
            this->loadCharacters();
        }

        void _deleteButtonPressed() {
            
            auto result = QMessageBox::warning(
                this, 
                "Suppression de fiche", 
                "Voulez-vous vraiment supprimer ce personnage ?",
                QMessageBox::Yes|QMessageBox::No, 
                QMessageBox::No
            );
            
            if(result == QMessageBox::Yes) {
                CharactersDatabase::get()->removeCharacter(this->_selectedCharacter);
                this->loadCharacters();
            }

        }

        void _onSelectedIndexChanged(int index) {
            this->_selectedCharacter = RPZCharacter(this->_characterListCombo->itemData(index).toHash());
            auto characterSelected = !this->_selectedCharacter.isEmpty();

            this->_deleteCharacterBtn->setEnabled(characterSelected);

            emit selectionChanged(this->_selectedCharacter);
        }
        
};