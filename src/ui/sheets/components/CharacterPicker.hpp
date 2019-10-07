#pragma once

#include <QWidget>
#include <QComboBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>

#include "src/shared/database/CharactersDatabase.hpp"

class CharacterPicker : public QWidget {

    Q_OBJECT

    signals:
        void selectionChanged(const RPZCharacter* selected);
        void requestSave(RPZCharacter* toSave);

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

        RPZCharacter* currentCharacter() {
            auto id = this->_characterListCombo->currentData().toULongLong();
            if(!this->_characters.contains(id)) return nullptr;
            return &this->_characters[id];
        }

        void loadCharacters() {
            
            //clear
            {
                QSignalBlocker b(this->_characterListCombo);
                this->_characterListCombo->clear();
            }
            this->_bufferedSelectedCharacter = nullptr;
            this->_bufferedSelectedIndex = -1;

            this->_characters = CharactersDatabase::get()->characters();
            
            //if no character in DB
            if(this->_characters.isEmpty()) {
                this->_characterListCombo->addItem("Aucun personnage existant, créez en un !");
                this->_characterListCombo->setEnabled(false);
                return;
            }

            //add an item for each
            this->_characterListCombo->setEnabled(true);
            {
                QSignalBlocker b(this->_characterListCombo);
                for(auto &character : this->_characters) {
                    this->_characterListCombo->addItem(this->_icon, character.toString(), character.id());
                }
            }

            //make sure to trigger event after refresh
            auto last = this->_characters.count() - 1;
            auto ci = this->_characterListCombo->currentIndex();
            if(last != ci) {
                this->_characterListCombo->setCurrentIndex(last);
            } else {
                this->_onSelectedIndexChanged(ci);
            }
            
        }

        void updateBufferedItemString() {
            if(!this->_bufferedSelectedCharacter) return;

            this->_characterListCombo->setItemText(
                this->_bufferedSelectedIndex,
                this->_bufferedSelectedCharacter->toString()
            );
        }
    
    private:
        QComboBox* _characterListCombo = nullptr;
        QPushButton* _deleteCharacterBtn = nullptr;
        RPZCharacter* _bufferedSelectedCharacter = nullptr;
        int _bufferedSelectedIndex = -1;
        QIcon _icon = QIcon(":/icons/app/connectivity/cloak.png");

        RPZMap<RPZCharacter> _characters;

        void _addButtonPressed() {
            this->_autoSave();
            CharactersDatabase::get()->addNewCharacter();
            this->loadCharacters();
        }

        void _deleteButtonPressed() {
            
            auto currentChar = this->currentCharacter();
            if(!currentChar) return;

            auto result = QMessageBox::warning(
                this, 
                "Suppression de fiche", 
                "Voulez-vous vraiment supprimer ce personnage ?",
                QMessageBox::Yes|QMessageBox::No, 
                QMessageBox::No
            );
            
            if(result == QMessageBox::Yes) {
                CharactersDatabase::get()->removeCharacter(*currentChar);
                this->_bufferedSelectedCharacter = nullptr;
                this->_bufferedSelectedIndex = -1;
                this->loadCharacters();
            }

        }

        bool _autoSave() {
            if(!this->_bufferedSelectedCharacter) return false;

            emit requestSave(this->_bufferedSelectedCharacter);
            
            this->updateBufferedItemString();

            return true;
        }

        void _onSelectedIndexChanged(int index) {
            this->_autoSave();

            auto newCharacter = this->currentCharacter();
            this->_deleteCharacterBtn->setEnabled(newCharacter);
            
            this->_bufferedSelectedCharacter = newCharacter;
            this->_bufferedSelectedIndex = index;

            emit selectionChanged(newCharacter);
        }
        
};