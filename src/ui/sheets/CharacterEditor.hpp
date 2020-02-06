#pragma once

#include <QWidget>
#include <QGroupBox>

#include "CharacterSheet.hpp"
#include "src/shared/database/CharactersDatabase.h"

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
                    this->_characterPicker, &CharacterPicker::requestSheetDisplay,
                    this, &CharacterEditor::_onRequestedSheetToDisplay
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
            
            QObject::connect(
                this->_saveCharacterBtn, &QPushButton::pressed,
                this, &CharacterEditor::_maySaveCurrentCharacter
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
            this->_characterPicker->setup();
            this->_defineTitle();
            this->_defineSavability();

        }

        ~CharacterEditor() {
            delete this->_characterPicker; //prevent infinite callbacks
            this->_maySaveCurrentCharacter();
        }
    
    public slots:
        void tryToSelectCharacter(const RPZCharacter::Id &characterIdToFocus) {
            auto success = this->_characterPicker->pickCharacter(characterIdToFocus);
            if(success) this->setFocus(Qt::OtherFocusReason);
        }

    protected:
        void connectingToServer() override {
            this->_defineTitle(true);
        }

        void connectionClosed(bool hasInitialMapLoaded) override {
            this->_defineTitle();
        }

    private:
        CharacterPicker::SelectedCharacter _currentSelection;

        CharacterPicker* _characterPicker = nullptr;
        CharacterSheet* _sheet = nullptr;
        QPushButton* _saveCharacterBtn = nullptr;
        QGroupBox* _characterPickerGrpBox = nullptr;

        void _maySaveCurrentCharacter() {
            
            if(!this->_isCurrentSelectionSavable()) return;

            //prevent save if is read only
            if(this->_sheet->isReadOnlyMode()) return;

            //make sure character have id
            auto characterFromSheet = this->_sheet->generateCharacter();
            if(!characterFromSheet.id()) return;

            //update DB
            CharactersDatabase::get()->updateCharacter(characterFromSheet);

            //if remote, tell server that character changed
            if(this->_rpzClient) {
                QMetaObject::invokeMethod(this->_rpzClient, "notifyCharacterChange", 
                    Q_ARG(RPZCharacter, characterFromSheet)
                );
            }
        }

        void _deleteRequestFromPicker(const RPZCharacter::Id &toDelete) {
            this->_currentSelection = {}; //back to default to trick the autosave into not saving it back
            CharactersDatabase::get()->removeCharacter(toDelete); 
        }

        void _insertRequestFromPicker() {
            CharactersDatabase::get()->addNewCharacter(); 
        }

        void _onRequestedSheetToDisplay(const CharacterPicker::SelectedCharacter &newSelection, const RPZCharacter &toDisplay) {
            
            //may save previous character
            this->_maySaveCurrentCharacter();

            //change selection and define savability
            this->_currentSelection = newSelection;
            auto isReadOnly = !this->_defineSavability();
            
            //load
            this->_sheet->loadCharacter(toDisplay, isReadOnly);

        }

        void _defineTitle(bool isRemote = false) {
            auto title = isRemote ? tr("Hosted characters") : tr("My characters");
            this->_characterPickerGrpBox->setTitle(title);
        }

        bool _isCurrentSelectionSavable() {
            return this->_currentSelection.id && this->_currentSelection.origin == CharacterPicker::CharacterOrigin::Local;
        }

        bool _defineSavability() {
            bool enableSave = this->_isCurrentSelectionSavable();
            this->_saveCharacterBtn->setVisible(enableSave);
            return enableSave;
        }

};