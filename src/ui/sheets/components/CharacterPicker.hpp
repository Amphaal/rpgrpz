#pragma once

#include <QWidget>
#include <QComboBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>

#include "src/shared/database/CharactersDatabase.h"

class CharacterPicker : public QWidget {

    Q_OBJECT

    signals:
        void selectionChanged(const RPZCharacter::Id &selectedId);
        void requestSave();
        void requestDelete(const RPZCharacter::Id &idToRemove);
        void requestInsert();

    public:
        enum class Mode { Unknown, Local, Remote };
        Q_ENUM(Mode)

        CharacterPicker() : 
            _characterListCombo(new QComboBox), 
            _deleteCharacterBtn(new QPushButton),
            _newCharacterBtn(new QPushButton) {

            //add new character
            this->_newCharacterBtn->setIcon(QIcon(QStringLiteral(u":/icons/app/other/add.png")));
            this->_newCharacterBtn->setToolTip(tr("Create a new sheet"));
            this->_newCharacterBtn->setMaximumWidth(25);
            QObject::connect(
                this->_newCharacterBtn, &QPushButton::pressed,
                this, &CharacterPicker::_addButtonPressed
            );

            //delete character
            this->_deleteCharacterBtn->setToolTip(tr("Delete sheet"));
            this->_deleteCharacterBtn->setMaximumWidth(25);
            this->_deleteCharacterBtn->setIcon(QIcon(QStringLiteral(u":/icons/app/other/remove.png")));
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

        const RPZCharacter::Id currentCharacterId() const {
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

        bool pickCharacter(const RPZCharacter::Id &characterIdToFocus) {

            auto indexItemToFocus = this->_getIndexOfCharacterId(characterIdToFocus);
            
            if(indexItemToFocus < 0) return false;
            if(indexItemToFocus == this->_characterListCombo->currentIndex()) return true;

            this->_characterListCombo->setCurrentIndex(indexItemToFocus);
            return true;

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

                auto emptyItemText = isLocal ? 
                    tr("No existing character, create some !") : 
                    tr("Waiting for sheets from host...");
                this->_characterListCombo->addItem(emptyItemText);
                this->_characterListCombo->setEnabled(false);
                
                this->_onCurrentCharacterIdChanged();
                return;

            }

            //add an item for each
            this->_characterListCombo->setEnabled(true);
            for(const auto &character : toLoad) {
                
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

        RPZCharacter::Id localCharacterIdFromRemote() {
            return this->_localCharacterIdFromRemote;
        }

        void setLocalCharacterIdFromRemote(RPZCharacter::Id localCharacterId) {
            this->_localCharacterIdFromRemote = localCharacterId;
        };
    
    private:
        RPZCharacter::Id _localCharacterIdFromRemote = 0;
        CharacterPicker::Mode _mode = CharacterPicker::Mode::Unknown;
        QVector<RPZCharacter::Id> _ids;

        QComboBox* _characterListCombo = nullptr;
        QPushButton* _deleteCharacterBtn = nullptr;
        QPushButton* _newCharacterBtn = nullptr;

        QIcon _standardClockIcon = QIcon(QStringLiteral(u":/icons/app/connectivity/cloak.png"));
        QIcon _selfCloakIcon = QIcon(QStringLiteral(u":/icons/app/connectivity/self_cloak.png"));

        void _addButtonPressed() {
            this->_autoSave();
            emit requestInsert();
        }

        void _deleteButtonPressed() {
            
            auto currentId = this->currentCharacterId();
            if(!currentId) return;

            auto result = QMessageBox::warning(
                this, 
                tr("Character sheet deletion"), 
                tr("Do you really want to delete this character ?"),
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
            
            if(this->_mode == Mode::Local) {
                emit requestSave();
            } 
            
            else if(this->_mode == Mode::Remote) {
                if(this->_localCharacterIdFromRemote != currentId) return;
                emit requestSave();
            }
            
        }

        void _onCurrentCharacterIdChanged() {
            auto id = this->currentCharacterId();
            this->_deleteCharacterBtn->setEnabled(id);
            emit selectionChanged(id);
        }

        int _getIndexOfCharacterId(const RPZCharacter::Id &characterIdToFind) {
            return this->_ids.indexOf(characterIdToFind);
        }
        
};