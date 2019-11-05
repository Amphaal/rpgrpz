#pragma once

#include <QLineEdit>
#include <QPushButton>

#include "src/ui/atomEditor/_base/AtomSubEditor.h"

class AtomShortTextEditor : public AtomSubEditor  {
    
    Q_OBJECT

    public:
        QLineEdit* lineEdit() {
            return (QLineEdit*)this->_dataEditor;
        }

        AtomShortTextEditor(const RPZAtom::Parameter &parameter) : AtomSubEditor({parameter}), 
            _validateButton(new QPushButton(this)) {
            
            this->_validateButton->setText(tr("Confirm modification"));
            
            auto edit = new QLineEdit(this);
            edit->setPlaceholderText(tr("Type some text..."));
            this->_setAsDataEditor(edit);

            this->layout()->addWidget(this->_validateButton);

            QObject::connect(
                this->_validateButton, &QPushButton::pressed,
                [&]() {
                    auto out = QVariant(this->lineEdit()->text());
                    emit valueConfirmedForPayload({{this->_params.first(), out}});
                }
            );

        }
    
    private:
        QPushButton* _validateButton = nullptr;

        const AtomSubEditor::FilteredDefaultValues loadTemplate(const RPZAtom::Updates &defaultValues, bool updateMode) override {
            
            auto filtered = AtomSubEditor::loadTemplate(defaultValues, updateMode);
            auto castedVal = filtered[this->_params.first()].toString();
            
            QSignalBlocker b(this->lineEdit());
            this->lineEdit()->setText(castedVal);

            return defaultValues;
            
        }
};