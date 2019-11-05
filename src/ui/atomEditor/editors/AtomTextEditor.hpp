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

        AtomTextEditor(const RPZAtom::Parameter &parameter) : AtomSubEditor({parameter}), 
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
                    emit valueConfirmedForPayload({{this->_params.first(), out}});
                }
            );

        }
    
    private:

        QPushButton* _validateButton = nullptr;

        const AtomSubEditor::FilteredDefaultValues loadTemplate(const RPZAtom::Updates &defaultValues, bool updateMode) override {
            
            auto filtered = AtomSubEditor::loadTemplate(defaultValues, updateMode);
            auto castedVal = filtered[this->_params.first()].toString();
            
            QSignalBlocker b(this->textEdit());
            this->textEdit()->setText(castedVal);

            return filtered;
            
        }
};