#pragma once

#include <QWidget>
#include <QGroupBox>

#include "CharacterSheet.hpp"
#include "src/shared/database/CharactersDatabase.h"

#include "src/ui/_others/ConnectivityObserver.h"

#include "src/ui/sheets/components/CharacterPicker.hpp"

class CharacterEditor : public QWidget, public ConnectivityObserver {
    
    Q_OBJECT

    public:
        CharacterEditor(QWidget *parent = nullptr) : QWidget(parent),
            _characterPicker(new CharacterPicker), 
            _sheet(new CharacterSheet), 
            _saveCharacterBtn(new QPushButton),
            _characterPickerGrpBox(new QGroupBox) {

            this->setLayoutDirection(Qt::LayoutDirection::LeftToRight);

            //picker        
                QObject::connect(
                    this->_characterPicker, &CharacterPicker::selectionChanged,
                    this, &CharacterEditor::_onSelectedCharacterChanged
                );
                QObject::connect(
                    this->_characterPicker, &CharacterPicker::requestSave,
                    this, &CharacterEditor::_saveCurrentCharacter
                );
                QObject::connect(
                    this->_characterPicker, &CharacterPicker::requestInsert,
                    this, &CharacterEditor::_insertRequestFromPicker
                );
                QObject::connect(
                    this->_characterPicker, &CharacterPicker::requestDelete,
                    this, &CharacterEditor::_deleteRequestFromPicker
                );

            //save character
            this->_saveCharacterBtn->setToolTip(tr("Save character sheet"));
            this->_saveCharacterBtn->setIcon(QIcon(QStringLiteral(u":/icons/app/other/save.png")));
            this->_saveCharacterBtn->setVisible(false);
            
            QObject::connect(
                this->_saveCharacterBtn, &QPushButton::pressed,
                this, &CharacterEditor::_saveCurrentCharacter
            );

            //layout
            auto l = new QVBoxLayout;
            this->setLayout(l);
            l->setAlignment(Qt::AlignTop);
            
            this->_characterPickerGrpBox->setLayout(new QVBoxLayout);
            this->_characterPickerGrpBox->layout()->setMargin(0);
            this->_characterPickerGrpBox->setAlignment(Qt::AlignCenter);
            this->_characterPickerGrpBox->layout()->addWidget(this->_characterPicker);

            l->addWidget(this->_characterPickerGrpBox);
            l->addWidget(this->_sheet);
            l->addWidget(this->_saveCharacterBtn, 0, Qt::AlignRight);

            //init
            this->_setMode(CharacterPicker::Mode::Local);

        }

        ~CharacterEditor() {
            this->_saveCurrentCharacter();
        }
    
    public slots:
        void tryToSelectCharacter(const SnowFlake::Id &characterIdToFocus) {
            this->_characterPicker->pickCharacter(characterIdToFocus);
            this->setFocus(Qt::OtherFocusReason);
        }

    protected:
        void connectingToServer() override {
            
            QObject::connect(
                this->_rpzClient, &RPZClient::allUsersReceived,
                this, &CharacterEditor::_onAllUsersReceived
            );

            QObject::connect(
                this->_rpzClient, &RPZClient::userJoinedServer,
                this, &CharacterEditor::_onUserJoinedServer
            );

            QObject::connect(
                this->_rpzClient, &RPZClient::userLeftServer,
                this, &CharacterEditor::_onUserLeftServer
            );

            QObject::connect(
                this->_rpzClient, &RPZClient::userDataChanged,
                this, &CharacterEditor::_onUserDataChanged
            );

            this->_setMode(CharacterPicker::Mode::Remote);

        }

        void connectionClosed(bool hasInitialMapLoaded) override {
            this->_setMode(CharacterPicker::Mode::Local);
        }

    private slots:
        void _onAllUsersReceived() {
            
            auto dbCharacterIds = CharactersDatabase::get()->characters().keys();
            
            RPZMap<RPZCharacter> out;
            for(auto &remoteUser : this->_rpzClient->sessionUsers()) {
                
                //reject if not player
                if(remoteUser.role() != RPZUser::Role::Player) continue;

                auto character = remoteUser.character();
                auto id = character.id();
                auto remoteCharacterIsInLocalDB = dbCharacterIds.contains(id);

                if(remoteCharacterIsInLocalDB) {
                    this->_characterPicker->setLocalCharacterIdFromRemote(id);
                }

                else {
                    out.insert(id, character);
                }

            }
            this->_remoteDb = out;

            this->_loadPickerCharactersFromRemote();

        }

        void _onUserJoinedServer(const RPZUser &newUser) {
            
            if(newUser.role() != RPZUser::Role::Player) return;

            auto character = newUser.character();
            this->_remoteDb.insert(character.id(), character);

            this->_loadPickerCharactersFromRemote();

        }

        void _onUserLeftServer(const RPZUser &userOut) {
            
            if(userOut.role() != RPZUser::Role::Player) return;

            this->_remoteDb.remove(userOut.character().id());

            this->_loadPickerCharactersFromRemote();

        }

        void _onUserDataChanged(const RPZUser &updatedUser) {
            
            if(updatedUser.role() != RPZUser::Role::Player) return;

            auto character = updatedUser.character();
            this->_remoteDb.insert(character.id(), character);

            this->_loadPickerCharactersFromRemote();

        }

    private:
        RPZMap<RPZCharacter> _remoteDb;

        CharacterPicker::Mode _mode = CharacterPicker::Mode::Unknown;

        QPushButton* _saveCharacterBtn = nullptr;
        CharacterPicker* _characterPicker = nullptr;
        CharacterSheet* _sheet = nullptr;
        QGroupBox* _characterPickerGrpBox = nullptr;

        void _saveCurrentCharacter() {
            
            //prevent save if is read only
            if(this->_sheet->isReadOnlyMode()) return;

            //make sure character have id
            auto characterFromSheet = this->_sheet->generateCharacter();
            if(!characterFromSheet.id()) return;

            //update DB
            CharactersDatabase::get()->updateCharacter(characterFromSheet);
            
            //update label
            this->_characterPicker->updateItemText(
                characterFromSheet
            );

            //if remote, tell server that character changed
            if(this->_mode == CharacterPicker::Mode::Remote) {
                QMetaObject::invokeMethod(this->_rpzClient, "notifyCharacterChange", 
                    Q_ARG(RPZCharacter, characterFromSheet)
                );
            }
        }

        void _deleteRequestFromPicker(const SnowFlake::Id toDelete) {
            
            if(this->_mode != CharacterPicker::Mode::Local) return;

            CharactersDatabase::get()->removeCharacter(toDelete); 
            this->_loadPickerCharactersFromDatabase();

        }

        void _insertRequestFromPicker() {
            
            if(this->_mode != CharacterPicker::Mode::Local) return;

            CharactersDatabase::get()->addNewCharacter(); 
            this->_loadPickerCharactersFromDatabase();

        }

        void _onSelectedCharacterChanged(const SnowFlake::Id selectedId) {
            
            bool enableSave = selectedId;

            auto loadCharacterFromDB = [=]() {
                this->_sheet->loadCharacter(
                    CharactersDatabase::get()->character(selectedId),
                    false
                );
            };

            auto loadCharacterFromRemote = [=]() {
                this->_sheet->loadCharacter(
                    this->_remoteDb.value(selectedId),
                    true
                );
            };

            switch(this->_mode) {
                
                case CharacterPicker::Mode::Local: {
                    loadCharacterFromDB();
                }
                break;

                case CharacterPicker::Mode::Remote: {
                    
                    enableSave = enableSave && this->_characterPicker->localCharacterIdFromRemote() == selectedId;
                    
                    if(enableSave) {
                        loadCharacterFromDB();
                    }

                    else {
                        loadCharacterFromRemote();
                    }

                }
                break;

                default:
                break;

            }         

            this->_saveCharacterBtn->setVisible(enableSave);

        }

        void _setMode(CharacterPicker::Mode mode) {

            //auto save
            this->_saveCurrentCharacter();

            //change current mode
            this->_mode = mode;

            //update grpBox label
            auto title = mode == CharacterPicker::Mode::Local ? tr("My characters") : tr("Hosted characters");
            this->_characterPickerGrpBox->setTitle(title);

            switch(mode) {

                case CharacterPicker::Mode::Local: {
                    
                    //sync load
                    this->_loadPickerCharactersFromDatabase();

                }
                break;

                case CharacterPicker::Mode::Remote: {
                   
                    //async load, empty in the inbetween
                    this->_characterPicker->unloadCharacters();
                    this->setEnabled(false); 

                }
                break;

                default:
                break;

            }

        }

        void _loadPickerCharactersFromDatabase() {
            
            this->_characterPicker->loadCharacters(
                CharactersDatabase::get()->characters(),
                CharacterPicker::Mode::Local
            );

            this->setEnabled(true);

        }

        void _loadPickerCharactersFromRemote() {
            
            auto local = CharactersDatabase::get()->character(
                this->_characterPicker->localCharacterIdFromRemote()
            );
            
            auto remoteCopy = this->_remoteDb;
            if(!local.isEmpty()) remoteCopy.insert(local.id(), local);

            this->_characterPicker->loadCharacters(
                remoteCopy,
                CharacterPicker::Mode::Remote,
                true
            );

            this->setEnabled(true);

        }
};