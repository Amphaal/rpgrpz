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
// different license and copyright still refer to this GNU General Public License.

#pragma once

#include "src/ui/atomEditor/_base/AtomEditorLineDescriptor.h"
#include <QVBoxLayout>
#include <QSpinBox>

#include "src/shared/models/RPZAtom.h"
#include "src/ui/atomEditor/_base/AtomSubEditor.h"
#include "src/ui/sheets/components/SimpleGaugeEditor.hpp"

class NPCHealthEditor : public AtomSubEditor {

    Q_OBJECT

    private:    
        SimpleGaugeEditor* _gEditor = nullptr;
        QPushButton* _validateButton = nullptr;

    public:
        NPCHealthEditor() : AtomSubEditor({RPZAtom::Parameter::NPCHealth, RPZAtom::Parameter::MinNPCHealth, RPZAtom::Parameter::MinNPCHealth}) { 
            
            this->_gEditor = new SimpleGaugeEditor;
            this->_gEditor->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

            this->_validateButton = new QPushButton(this);
            this->_validateButton->setText(tr("Confirm modification"));

            QObject::connect(
                this->_validateButton, &QPushButton::pressed,
                [=]() {
                    
                    auto values = this->_gEditor->toMinimalist();
                    
                    RPZAtom::Updates updates {
                        { RPZAtom::Parameter::NPCHealth, values.current },
                        { RPZAtom::Parameter::MinNPCHealth, values.min },
                        { RPZAtom::Parameter::MaxNPCHealth, values.max }
                    };

                    emit valueConfirmedForPayload(updates);

                }
            );

            this->setVisible(false);
            this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
            
            this->_mainLayout->addWidget(this->_gEditor, 0, Qt::AlignCenter);
            this->_mainLayout->addWidget(this->_validateButton);

        };

        void loadTemplate(const RPZAtom::Updates &defaultValues, const AtomSubEditor::LoadingContext &context) override {

            AtomSubEditor::loadTemplate(defaultValues, context);

            this->_gEditor->fillValues({
                defaultValues.value(RPZAtom::Parameter::NPCHealth).toInt(),
                defaultValues.value(RPZAtom::Parameter::MinNPCHealth).toInt(),
                defaultValues.value(RPZAtom::Parameter::MaxNPCHealth).toInt(),
            });
 
        }

};