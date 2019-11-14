#pragma once

#include <QLineEdit>
#include <QPushButton>

#include "src/ui/atomEditor/_base/AtomSubEditor.h"

class AtomShortTextEditor : public AtomSubEditor {
    
    Q_OBJECT

    public:
        AtomShortTextEditor(const RPZAtom::Parameter &parameter) : AtomSubEditor({parameter}), 
            _validateButton(new QPushButton(this)) {
            
            this->_validateButton->setText(tr("Confirm modification"));
            
            this->_edit = new QLineEdit(this);
            this->_edit->setPlaceholderText(tr("Type some text..."));

            this->layout()->addWidget(this->_edit);
            this->layout()->addWidget(this->_validateButton);

            QObject::connect(
                this->_validateButton, &QPushButton::pressed,
                [&]() {
                    auto out = QVariant(this->_edit->text());
                    emit valueConfirmedForPayload({{this->_params.first(), out}});
                }
            );

        }
    
    private:
        QLineEdit* _edit = nullptr;
        QPushButton* _validateButton = nullptr;

        void loadTemplate(const RPZAtom::Updates &defaultValues, const AtomSubEditor::EditMode &editMode) override {
            
            AtomSubEditor::loadTemplate(defaultValues, editMode);
            auto castedVal = defaultValues[this->_params.first()].toString();
            
            QSignalBlocker b(this->_edit);
            this->_edit->setText(castedVal);
            
        }
};