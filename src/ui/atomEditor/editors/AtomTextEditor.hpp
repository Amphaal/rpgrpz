#pragma once

#include <QTextEdit>
#include <QPushButton>

#include "src/ui/atomEditor/_base/AtomSubEditor.h"

class AtomTextEditor : public AtomSubEditor  {
    
    Q_OBJECT

    public:
        QTextEdit* textEdit() {
            return (QTextEdit*)this->_dataEditor;
        }

        AtomTextEditor(const AtomParameter &parameter) : AtomSubEditor(parameter), 
            _validateButton(new QPushButton(this)) {
            
            this->_validateButton->setText(tr("Confirm modification"));

            auto edit = new QTextEdit(this);
            edit->setPlaceholderText(tr("Type some text..."));
            this->_setAsDataEditor(edit);

            this->layout()->addWidget(this->_validateButton);

            QObject::connect(
                this->_validateButton, &QPushButton::pressed,
                [&]() {
                    auto out = QVariant(this->textEdit()->toPlainText());
                    emit valueConfirmedForPayload(this->_param, out);
                }
            );

        }
    
    private:

        QPushButton* _validateButton = nullptr;

        QVariant loadTemplate(const AtomUpdates &defaultValues, bool updateMode) override {
            
            auto defaultValue = AtomSubEditor::loadTemplate(defaultValues, updateMode);
            auto castedVal = defaultValue.toString();
            
            QSignalBlocker b(this->textEdit());
            this->textEdit()->setText(castedVal);

            return defaultValue;
            
        }
};