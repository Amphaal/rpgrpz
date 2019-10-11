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
        void selectionChanged(const snowflake_uid selectedId);
        void requestSave();
        void requestDelete(const snowflake_uid idToRemove);
        void requestInsert();

    public:
        enum Mode { Unknown, Local, Remote };

        CharacterPicker() : 
            _characterListCombo(new QComboBox), 
            _deleteCharacterBtn(new QPushButton),
            _newCharacterBtn(new QPushButton) {

            //add new character
            this->_newCharacterBtn->setIcon(QIcon(":/icons/app/other/add.png"));
            this->_newCharacterBtn->setToolTip("Créer une nouvelle fiche");
            this->_newCharacterBtn->setMaximumWidth(25);
            QObject::connect(
                this->_newCharacterBtn, &QPushButton::pressed,
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
            this->setContentsMargins(0, 0, 0, 0);
            clLayout->addWidget(this->_characterListCombo, 1);
            clLayout->addWidget(this->_deleteCharacterBtn);
            clLayout->addWidget(this->_newCharacterBtn);

        }

        const snowflake_uid currentCharacterId() const {
            return this->_characterListCombo->currentData().toULongLong();
        }

        void updateItemText(const RPZCharacter &updatedCharacter) {
            
            auto indexItemToUpdate = this->_getIndexOfCharacterId(updatedCharacter.id());
            if(indexItemToUpdate < 0) return;

            this->_characterListCombo->setItemText(
                indexItemToUpdate, 
                updatedCharacter.toString()
            );

        }

        void unloadCharacters() {
            this->setLocalCharacterIdFromRemote(0);
            this->loadCharacters({}, CharacterPicker::Mode::Remote);
        }

        void pickCharacter(const snowflake_uid &characterIdToFocus) {

            auto indexItemToFocus = this->_getIndexOfCharacterId(characterIdToFocus);
            
            if(indexItemToFocus < 0) return;
            if(indexItemToFocus == this->_characterListCombo->currentIndex()) return;

            this->_characterListCombo->setCurrentIndex(indexItemToFocus);

        }

        void loadCharacters(const RPZMap<RPZCharacter> &toLoad, CharacterPicker::Mode mode, bool maintainSelection = false) {
            
            //update mode
            this->_mode = mode;
            auto isLocal = mode == CharacterPicker::Mode::Local;
            this->_newCharacterBtn->setVisible(isLocal); 
            this->_deleteCharacterBtn->setVisible(isLocal); 

            //store previous selection            
            auto previousSelectedId = this->currentCharacterId();
            
            //clear
            QSignalBlocker b(this->_characterListCombo);
            this->_characterListCombo->clear();
            this->_ids.clear();
            
            //if nothing to load
            if(toLoad.isEmpty()) {

                auto emptyItemText = isLocal ? "Aucun personnage existant, créez en un !" : "En Attente de fiches depuis le serveur...";
                this->_characterListCombo->addItem(emptyItemText);
                this->_characterListCombo->setEnabled(false);
                
                this->_onCurrentCharacterIdChanged();
                return;

            }

            //add an item for each
            this->_characterListCombo->setEnabled(true);
            for(auto &character : toLoad) {
                
                auto id = character.id();
                auto futureInsertIndex = this->_characterListCombo->count();

                this->_ids += id;

                //pick icon
                QIcon* iconToUse;
                if(isLocal) iconToUse = &this->_selfCloakIcon;
                else iconToUse = this->_localCharacterIdFromRemote == id ? &this->_selfCloakIcon : &this->_standardClockIcon;
                
                //add item
                this->_characterListCombo->addItem(
                    *iconToUse, 
                    character.toString(), 
                    character.id()
                );

                //default selection
                if(maintainSelection && id == previousSelectedId) {
                    this->_characterListCombo->setCurrentIndex(futureInsertIndex);
                }

            }
            
            //signal that picker changed
            this->_onCurrentCharacterIdChanged();

        }

        snowflake_uid localCharacterIdFromRemote() {
            return this->_localCharacterIdFromRemote;
        }

        void setLocalCharacterIdFromRemote(snowflake_uid localCharacterId) {
            this->_localCharacterIdFromRemote = localCharacterId;
        };
    
    private:
        snowflake_uid _localCharacterIdFromRemote = 0;
        CharacterPicker::Mode _mode = CharacterPicker::Mode::Unknown;
        QVector<snowflake_uid> _ids;

        QComboBox* _characterListCombo = nullptr;
        QPushButton* _deleteCharacterBtn = nullptr;
        QPushButton* _newCharacterBtn = nullptr;

        QIcon _standardClockIcon = QIcon(":/icons/app/connectivity/cloak.png");
        QIcon _selfCloakIcon = QIcon(":/icons/app/connectivity/self_cloak.png");

        void _addButtonPressed() {
            this->_autoSave();
            emit requestInsert();
        }

        void _deleteButtonPressed() {
            
            auto currentId = this->currentCharacterId();
            if(!currentId) return;

            auto result = QMessageBox::warning(
                this, 
                "Suppression de fiche", 
                "Voulez-vous vraiment supprimer ce personnage ?",
                QMessageBox::Yes|QMessageBox::No, 
                QMessageBox::No
            );
            
            if(result == QMessageBox::Yes) {
                emit requestDelete(currentId);
            }

        }
        
        void _onSelectedIndexChanged(int index) {
            this->_autoSave();
            this->_onCurrentCharacterIdChanged();
        }
        
        //

        void _autoSave() {
            auto currentId = this->currentCharacterId();
            if(!currentId) return;
            
            if(this->_mode == Local) {
                emit requestSave();
            } 
            
            else if(this->_mode == Remote) {
                if(this->_localCharacterIdFromRemote != currentId) return;
                emit requestSave();
            }
            
        }

        void _onCurrentCharacterIdChanged() {
            auto id = this->currentCharacterId();
            this->_deleteCharacterBtn->setEnabled(id);
            emit selectionChanged(id);
        }

        int _getIndexOfCharacterId(const snowflake_uid &characterIdToFind) {
            return this->_ids.indexOf(characterIdToFind);
        }
        
};