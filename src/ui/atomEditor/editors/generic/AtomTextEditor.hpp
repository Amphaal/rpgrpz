// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical or audio resources available within the source code may
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GPL.

#pragma once

#include <QTextEdit>
#include <QPushButton>

#include "src/ui/atomEditor/_base/AtomSubEditor.h"

class AtomTextEditor : public AtomSubEditor {
    
    Q_OBJECT

 public:
        AtomTextEditor(const RPZAtom::Parameter &parameter) : AtomSubEditor({parameter}), 
            _validateButton(new QPushButton(this)) {
            
            this->_validateButton->setText(tr("Confirm modification"));

            this->_edit = new QTextEdit(this);
            this->_edit->setPlaceholderText(tr("Type some text..."));
            this->_edit->setAcceptRichText(false);

            this->layout()->addWidget(this->_edit);
            this->layout()->addWidget(this->_validateButton);

            QObject::connect(
                this->_validateButton, &QPushButton::pressed,
                [&]() {
                    auto out = QVariant(this->_edit->toPlainText());
                    emit valueConfirmedForPayload({{this->_params.first(), out}});
                }
            );

        }
    
 private:
        QTextEdit* _edit = nullptr;
        QPushButton* _validateButton = nullptr;

        void loadTemplate(const RPZAtom::Updates &defaultValues, const AtomSubEditor::LoadingContext &context) override {
            
            AtomSubEditor::loadTemplate(defaultValues, context);
            auto castedVal = defaultValues[this->_params.first()].toString();
            
            QSignalBlocker b(this->_edit);
            this->_edit->setText(castedVal);
            
        }
};