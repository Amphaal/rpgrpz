#pragma once

#include <QComboBox>

#include "src/ui/atomEditor/_base/AtomSubEditor.h"

class CharacterPickerEditor : public AtomSubEditor {

    Q_OBJECT

    private:    
        QComboBox* _combo = nullptr;

        void _fillCombo(const RPZAtom::Updates &defaultValues) {
            QSignalBlocker b(this->_combo);

            this->_combo->clear();
            this->_combo->addItem(tr("[No character]"), 0); //no character, default

            auto characterId = defaultValues.value(this->_params.first()).toULongLong();
            
            if(characterId) {
                auto name = defaultValues.value(this->_params.last()).toString();
                this->_combo->addItem(QIcon(":/icons/app/connectivity/cloak.png"), name, characterId);
                this->_combo->setCurrentIndex(this->_combo->count() - 1);
            }

            else {
                this->_combo->setCurrentIndex(0);
            }

        }

    public:
        CharacterPickerEditor() : AtomSubEditor({RPZAtom::Parameter::CharacterId, RPZAtom::Parameter::CharacterName}, false) { 

            this->setVisible(false);

            this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
            
            this->_combo = new QComboBox;
            
            QObject::connect(
                this->_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                [&](int currentIndex) {
                    auto characterId = this->_combo->currentData();
                    emit valueConfirmedForPayload({{this->_params.first(), characterId}});
                }
            );

            this->layout()->addWidget(this->_combo);

        };

        void loadTemplate(const RPZAtom::Updates &defaultValues, const AtomSubEditor::EditMode &editMode) override {
            
            AtomSubEditor::loadTemplate(defaultValues, editMode);

            this->_fillCombo(defaultValues);

            this->_combo->setEnabled(false);
            
        }

};