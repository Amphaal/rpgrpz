#pragma once

#include <QWidget>
#include <QComboBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>

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

            //init
            this->_initLoad();

        }

        const SelectedCharacter currentCharacter() const {
            return { 
                this->_characterListCombo->currentData().toULongLong(),
                (CharacterOrigin)this->_characterListCombo->currentData(257).toInt(),
            };
        }

        void updateCharacterDescription(const RPZCharacter &updatedCharacter) {
            
            auto indexItemToUpdate = this->_getIndexOfCharacterId(updatedCharacter.id());
            if(indexItemToUpdate < 0) return;

            this->_characterListCombo->setItemText(
                indexItemToUpdate, 
                updatedCharacter.toString()
            );

        }

        bool pickCharacter(const RPZCharacter::Id &characterIdToFocus) {

            auto indexItemToFocus = this->_getIndexOfCharacterId(characterIdToFocus);
            
            if(indexItemToFocus < 0) return false;
            if(indexItemToFocus == this->_characterListCombo->currentIndex()) return true;

            this->_characterListCombo->setCurrentIndex(indexItemToFocus);
            return true;

        }
    
    signals:
        void selectionChanged(const CharacterPicker::SelectedCharacter &newSelection);
        void requestCharacterDeletion(const RPZCharacter::Id &idToRemove);
        void requestNewCharacter();

    protected:
        void connectingToServer() override {

            this->_loadPlaceholder(true);
            this->_updateDeletability();
            this->_updateInsertability();

            QObject::connect(
                this->_rpzClient, &RPZClient::gameSessionReceived,
                this, &CharacterPicker::_loadFromRemote
            );

        }

        void connectionClosed(bool hasInitialMapLoaded) override {
            this->_initLoad();
        }


    private:
        QComboBox* _characterListCombo = nullptr;
        QPushButton* _deleteCharacterBtn = nullptr;
        QPushButton* _newCharacterBtn = nullptr;

        QIcon _standardClockIcon = QIcon(QStringLiteral(u":/icons/app/connectivity/cloak.png"));
        QIcon _selfCloakIcon = QIcon(QStringLiteral(u":/icons/app/connectivity/self_cloak.png"));

        void _loadPlaceholder(bool fromRemote = false) {
            
            //clear and disable
            QSignalBlocker b(this->_characterListCombo);
            this->_characterListCombo->clear();
            this->_characterListCombo->setEnabled(false);
            
            //placeholder
            auto text = fromRemote ? tr("Waiting for sheets from host...") :  tr("No existing character, create some !");
            this->_characterListCombo->addItem(text);
            
            //emit
            this->_selectionChanged();

        }

        void _addItem(const RPZCharacter &characterToAdd, const CharacterOrigin &origin) {
                
            this->_characterListCombo->addItem(
                origin == CharacterOrigin::Local ? this->_selfCloakIcon : this->_standardClockIcon, 
                characterToAdd.toString(), 
                characterToAdd.id()
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
            if(!toLoad.count()) return _loadPlaceholder(true);

            //clear and enable
            QSignalBlocker b(this->_characterListCombo);
            this->_characterListCombo->clear();
            this->_characterListCombo->setEnabled(true);

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
            
            if(preferedSelectionIndex > -1) {
                this->_characterListCombo->setCurrentIndex(preferedSelectionIndex);
            }

            //signal that picker changed
            this->_selectionChanged();

        }

        void _loadLocalCharacters() {
            
            //maybe load placeholder
            auto toLoad = CharactersDatabase::get()->characters();
            if(!toLoad.count()) return _loadPlaceholder();

            //clear and enable
            QSignalBlocker b(this->_characterListCombo);
            this->_characterListCombo->clear();
            this->_characterListCombo->setEnabled(true);

            //add an item for each
            for(const auto &character : toLoad) {
                this->_addItem(character, CharacterOrigin::Local);
            }
            
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
            emit selectionChanged(cc);
        }


        int _getIndexOfCharacterId(const RPZCharacter::Id &characterIdToFind) {
            return this->_characterListCombo->findData(characterIdToFind);
        }
        
};