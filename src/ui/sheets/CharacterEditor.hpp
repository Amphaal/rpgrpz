#pragma once

#include <QWidget>
#include <QGroupBox>

#include "CharacterSheet.hpp"
#include "src/shared/database/CharactersDatabase.h"

#include "src/ui/sheets/components/CharacterPicker.hpp"

class CharacterEditor : public QWidget {
    
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
                    this->_characterPicker, &CharacterPicker::requestSheetUpdate,
                    this, &CharacterEditor::_onSelectedCharacterChanged
                );
                QObject::connect(
                    this->_characterPicker, &CharacterPicker::requestNewCharacter,
                    this, &CharacterEditor::_insertRequestFromPicker
                );
                QObject::connect(
                    this->_characterPicker, &CharacterPicker::requestCharacterDeletion,
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
        void tryToSelectCharacter(const RPZCharacter::Id &characterIdToFocus) {
            auto success = this->_characterPicker->pickCharacter(characterIdToFocus);
            if(success) this->setFocus(Qt::OtherFocusReason);
        }

    private:
        CharacterPicker::SelectedCharacter _currentSelection;

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

            //if remote, tell server that character changed
            if(this->_mode == CharacterPicker::Mode::Remote) {
                QMetaObject::invokeMethod(this->_rpzClient, "notifyCharacterChange", 
                    Q_ARG(RPZCharacter, characterFromSheet)
                );
            }
        }

        void _deleteRequestFromPicker(const RPZCharacter::Id &toDelete) {
            
            if(this->_mode != CharacterPicker::Mode::Local) return;

            CharactersDatabase::get()->removeCharacter(toDelete); 
            this->_loadPickerCharactersFromDatabase();

        }

        void _insertRequestFromPicker() {
            
            if(this->_mode != CharacterPicker::Mode::Local) return;

            CharactersDatabase::get()->addNewCharacter(); 
            this->_loadPickerCharactersFromDatabase();

        }

        void _onSelectedCharacterChanged(const RPZCharacter::Id &selectedId) {
            
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

};