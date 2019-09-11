#pragma once

#include <QTextEdit>
#include <QPushButton>

#include "src/ui/atoms/base/AtomSubEditor.h"

class AtomTextEditor : public AtomSubEditor  {
    public:

        QTextEdit* textEdit() {
            return (QTextEdit*)this->_dataEditor;
        }

        AtomTextEditor(const AtomParameter &parameter) : AtomSubEditor(parameter), _validateButton(new QPushButton("Valider la modification")) {
            
            this->_setAsDataEditor(new QTextEdit(this));

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

        void loadTemplate(QVector<const RPZAtom*> &atomsToManipulate, const QVariant &defaultValue) override {
            
            AtomSubEditor::loadTemplate(atomsToManipulate, defaultValue);

            auto castedVal = defaultValue.toString();
            
            this->textEdit()->blockSignals(true); 
                this->textEdit()->setText(castedVal);
            this->textEdit()->blockSignals(false);

        }
};