// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical resources available within the source code may 
// use a different license and copyright : please refer to their metadata
// for further details. Graphical resources without explicit references to a
// different license and copyright still refer to this GNU General Public License.

#pragma once

#include <QWidget>
#include <QComboBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>

#include "src/ui/_others/ConnectivityObserver.h"

#include "src/shared/database/CharactersDatabase.h"

class CharacterPicker : public QWidget, public ConnectivityObserver {

    Q_OBJECT

    public:
        enum class CharacterOrigin { Unknown, Local, Remote };
        Q_ENUM(CharacterOrigin)

        struct SelectedCharacter {
            RPZCharacter::Id id;
            CharacterOrigin origin;
        };

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

        void setup() {
            this->_handleLocalDbEvents();
            this->_initLoad();
        }

        const SelectedCharacter currentCharacter() const {
            return { 
                this->_characterListCombo->currentData().toULongLong(),
                (CharacterOrigin)this->_characterListCombo->currentData(257).toInt(),
            };
        }

        bool pickCharacter(const RPZCharacter::Id &characterIdToFocus) {

            auto indexItemToFocus = this->_getIndexOfCharacterId(characterIdToFocus);
            
            if(indexItemToFocus < 0) return false;
            if(indexItemToFocus == this->_characterListCombo->currentIndex()) return true;

            this->_characterListCombo->setCurrentIndex(indexItemToFocus);
            return true;

        }
    
    signals:
        void requestSheetDisplay(const CharacterPicker::SelectedCharacter &newSelection, const RPZCharacter &toDisplay);
        void requestCharacterDeletion(const RPZCharacter::Id &idToRemove);
        void requestNewCharacter();

    protected:
        void connectingToServer() override {

            this->_loadEmpty();
            this->_updateDeletability();
            this->_updateInsertability();

            QObject::connect(
                this->_rpzClient, &RPZClient::gameSessionReceived,
                this, &CharacterPicker::_loadFromRemote
            );

            QObject::connect(
                this->_rpzClient, &RPZClient::userJoinedServer,
                this, &CharacterPicker::_onUserJoinedServer
            );

            QObject::connect(
                this->_rpzClient, &RPZClient::userLeftServer,
                this, &CharacterPicker::_onUserLeftServer
            );

            QObject::connect(
                this->_rpzClient, &RPZClient::userDataChanged,
                this, &CharacterPicker::_onUserDataChanged
            );

        }

        void connectionClosed(bool hasInitialMapLoaded) override {
            this->_initLoad();
        }

    private slots:
        void _updateCharacter(const RPZCharacter &updatedCharacter) {
            
            //try to find item
            auto indexItemToUpdate = this->_getIndexOfCharacterId(updatedCharacter.id());
            if(indexItemToUpdate < 0) return;

            //update descr
            this->_characterListCombo->setItemText(
                indexItemToUpdate, 
                updatedCharacter.toString()
            );
            
            //prevent recurse if local update
            auto cc = this->currentCharacter();
            if(cc.origin == CharacterOrigin::Local) return;

            //if current selection, request sheet update
            if(cc.id == updatedCharacter.id()) {
                emit requestSheetDisplay(cc, updatedCharacter);
            }

        }

        void _removeCharacterId(const RPZCharacter::Id &removedId) {
            
            //find index of item
            auto index = this->_getIndexOfCharacterId(removedId);
            if(index < 0) return;

            //remove
            this->_characterListCombo->removeItem(index);

            //may display
            this->_mayTooglePlaceholder();

        }

        void _onUserJoinedServer(const RPZUser &newUser) {
            
            if(newUser.role() != RPZUser::Role::Player) return;
            
            //may remove
            this->_mayTooglePlaceholder();
            
            auto selectedIndex = this->_characterListCombo->currentIndex();

            //add item
            this->_addItem(
                newUser.character(), 
                CharacterOrigin::Remote
            );

            if(selectedIndex != this->_characterListCombo->currentIndex()) this->_selectionChanged();

        }

        void _onUserLeftServer(const RPZUser &userOut) {
            
            if(userOut.role() != RPZUser::Role::Player) return;

            //remove
            this->_removeCharacterId(userOut.character().id());

        }

        void _onUserDataChanged(const RPZUser &updatedUser) {
            
            if(updatedUser.role() != RPZUser::Role::Player) return;

            //update character
            auto character = updatedUser.character();
            this->_updateCharacter(character);
            
        }


    private:
        QComboBox* _characterListCombo = nullptr;
        QPushButton* _deleteCharacterBtn = nullptr;
        QPushButton* _newCharacterBtn = nullptr;

        QIcon _standardClockIcon = QIcon(QStringLiteral(u":/icons/app/connectivity/cloak.png"));
        QIcon _selfCloakIcon = QIcon(QStringLiteral(u":/icons/app/connectivity/self_cloak.png"));

        void _handleLocalDbEvents() {

            QObject::connect(
                CharactersDatabase::get(), &CharactersDatabase::characterAdded,
                [=](const RPZCharacter &added) {
                    
                    this->_addItemAndPick(
                        added, 
                        CharacterOrigin::Local
                    );

                }
            );

            QObject::connect(
                CharactersDatabase::get(), &CharactersDatabase::characterRemoved,
                this, &CharacterPicker::_removeCharacterId
            );

            QObject::connect(
                CharactersDatabase::get(), &CharactersDatabase::characterUpdated,
                this, &CharacterPicker::_updateCharacter
            );

        }

        void _mayTooglePlaceholder() {
            
            QSignalBlocker b(this->_characterListCombo);

            //remove placeholder
            auto itemCount = this->_characterListCombo->count();
            auto currentId = this->_characterListCombo->currentData().toULongLong();
            if(itemCount == 1 && currentId == 0) {
                this->_characterListCombo->removeItem(0);
                this->_characterListCombo->setEnabled(true); //premptive enabling
            }

            //add placeholder
            else if(itemCount == 0) {
                auto text = this->_rpzClient ? tr("Waiting for sheets from host...") : tr("No existing character, create some !");
                this->_characterListCombo->addItem(text);
                this->_characterListCombo->setEnabled(false);
            }

            //else, just make sure the combo is active
            else {
                this->_characterListCombo->setEnabled(true);
            }

        }

        void _loadEmpty() {
            
            {
                //clear and disable
                QSignalBlocker b(this->_characterListCombo);
                this->_characterListCombo->clear();
            }

            //placeholder
            this->_mayTooglePlaceholder();
            
            //emit
            this->_selectionChanged();

        }

        void _addItemAndPick(const RPZCharacter &characterToAdd, const CharacterOrigin &origin) {
                    
            //may remove placeholder
            this->_mayTooglePlaceholder();

            QSignalBlocker b(this->_characterListCombo);
            
            //add item
            this->_addItem(characterToAdd, origin);
            
            //pick
            auto latestInsertIndex = this->_characterListCombo->count() - 1;
            this->_characterListCombo->setCurrentIndex(latestInsertIndex);
            this->_selectionChanged();
            
        }

        void _addItem(const RPZCharacter &characterToAdd, const CharacterOrigin &origin) {
                
            this->_characterListCombo->addItem(
                origin == CharacterOrigin::Local ? this->_selfCloakIcon : this->_standardClockIcon, 
                characterToAdd.toString(), 
                QVariant::fromValue<RPZCharacter::Id>(characterToAdd.id())
            );

            this->_characterListCombo->setItemData(
                this->_characterListCombo->count() - 1, 
                (int)origin, 
                257
            );

        }

        void _initLoad() {
            this->_updateDeletability();
            this->_updateInsertability();
            this->_loadLocalCharacters();
        }

        void _loadFromRemote(const RPZGameSession &gameSession) {

            Q_UNUSED(gameSession);
            
            //maybe load placeholder
            auto toLoad = this->_rpzClient->sessionCharacters();
            if(!toLoad.count()) return this->_loadEmpty();

            //clear and enable
            QSignalBlocker b(this->_characterListCombo);
            this->_characterListCombo->clear();

            //find client character id
            auto myCharacterId = this->_rpzClient->identity().character().id();

            int preferedSelectionIndex = -1;

            //add an item for each
            for(const auto &pair : toLoad) {
                
                auto character = pair.second;

                //determine origin
                auto origin = CharacterOrigin::Remote;
                if(myCharacterId == character.id()) {
                    origin = CharacterOrigin::Local;
                    preferedSelectionIndex = this->_characterListCombo->count();
                }
                
                this->_addItem(character, origin);

            }
            
            //default selection if any
            if(preferedSelectionIndex > -1) {
                this->_characterListCombo->setCurrentIndex(preferedSelectionIndex);
            }

            //placeholder
            this->_mayTooglePlaceholder();

            //signal that picker changed
            this->_selectionChanged();

        }

        void _loadLocalCharacters() {
            
            //maybe load placeholder
            auto toLoad = CharactersDatabase::get()->characters();
            if(!toLoad.count()) return this->_loadEmpty();

            //clear and enable
            QSignalBlocker b(this->_characterListCombo);
            this->_characterListCombo->clear();

            //add an item for each
            for(const auto &character : toLoad) {
                this->_addItem(character, CharacterOrigin::Local);
            }

            //placeholder
            this->_mayTooglePlaceholder();
            
            //signal that picker changed
            this->_selectionChanged();

        }

        void _addButtonPressed() {
            emit requestNewCharacter();
        }

        bool _isDeletable(const CharacterPicker::SelectedCharacter &selection) {
            return !this->_rpzClient && selection.id && selection.origin == CharacterOrigin::Local;
        }

        void _deleteButtonPressed() {
            
            auto cc = this->currentCharacter();
            if(!this->_isDeletable(cc)) return;

            auto result = QMessageBox::warning(
                this, 
                tr("Character sheet deletion"), 
                tr("Do you really want to delete this character ?"),
                QMessageBox::Yes|QMessageBox::No, 
                QMessageBox::No
            );
            
            if(result == QMessageBox::Yes) {
                emit requestCharacterDeletion(cc.id);
            }

        }

        void _updateDeletability() {
            this->_updateDeletability(this->currentCharacter());
        }
        void _updateDeletability(const CharacterPicker::SelectedCharacter &cc) {
            this->_deleteCharacterBtn->setVisible(!this->_rpzClient); 
            this->_deleteCharacterBtn->setEnabled(this->_isDeletable(cc));
        }

        void _updateInsertability() {
            this->_newCharacterBtn->setVisible(!this->_rpzClient); 
        }
        
        void _onSelectedIndexChanged(int index) {
            this->_selectionChanged();
        }

        void _selectionChanged() {

            auto cc = this->currentCharacter();
            this->_updateDeletability(cc);

            emit requestSheetDisplay(cc, this->_getCharacter(cc));

        }

        const RPZCharacter _getCharacter(const SelectedCharacter &cc) {
            
            //empty
            if(!cc.id || cc.origin == CharacterOrigin::Unknown) return RPZCharacter();
            if(cc.origin == CharacterOrigin::Remote && !this->_rpzClient) return RPZCharacter();
            
            //local
            if(cc.origin == CharacterOrigin::Local) return CharactersDatabase::get()->character(cc.id);

            //remote
            if(cc.origin == CharacterOrigin::Remote) return this->_rpzClient->sessionCharacter(cc.id);

            //default
            return RPZCharacter();

        }

        int _getIndexOfCharacterId(const RPZCharacter::Id &characterIdToFind) {
            return this->_characterListCombo->findData(QVariant::fromValue<RPZCharacter::Id>(characterIdToFind));
        }
        
};