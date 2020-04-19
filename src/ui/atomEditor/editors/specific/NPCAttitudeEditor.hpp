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

#include "src/ui/atomEditor/_base/AtomEditorLineDescriptor.h"
#include <QVBoxLayout>
#include <QComboBox>

#include "src/shared/models/RPZAtom.h"
#include "src/ui/atomEditor/_base/AtomSubEditor.h"

class NPCAttitudeEditor : public AtomSubEditor {

    Q_OBJECT

 private:    
        static inline QMap<RPZAtom::NPCType, QString> _strAttitude {
            { RPZAtom::NPCType::Unknown, QT_TR_NOOP("Unknown") },
            { RPZAtom::NPCType::Neutral, QT_TR_NOOP("Neutral") },
            { RPZAtom::NPCType::Friendly, QT_TR_NOOP("Friendly") },
            { RPZAtom::NPCType::Hostile, QT_TR_NOOP("Hostile") },
        };

        static inline QHash<RPZAtom::NPCType, QString> _AttitudeIcons {
            { RPZAtom::NPCType::Unknown, QStringLiteral(u":/icons/app/attitude/unknown.png") },
            { RPZAtom::NPCType::Neutral, QStringLiteral(u":/icons/app/attitude/neutral.png") },
            { RPZAtom::NPCType::Friendly, QStringLiteral(u":/icons/app/attitude/friendly.png") },
            { RPZAtom::NPCType::Hostile, QStringLiteral(u":/icons/app/attitude/hostile.png") }
        };

        QComboBox* _combo = nullptr;

 public:
        NPCAttitudeEditor() : AtomSubEditor({RPZAtom::Parameter::NPCAttitude}) { 

            this->setVisible(false);

            this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
            
            this->_combo = new QComboBox;
            
            for(auto i = _strAttitude.constBegin(); i != _strAttitude.constEnd(); i++) {

                auto tool = i.key();
                
                auto translatedName = tr(qUtf8Printable(i.value()));
                auto associatedIcon = QIcon(_AttitudeIcons.value(tool));

                this->_combo->addItem(associatedIcon, translatedName, (int)tool);

            }

            QObject::connect(
                this->_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                [&](int currentIndex) {
                    auto out = this->_combo->currentData();
                    emit valueConfirmedForPayload({{this->_params.first(), out}});
                }
            );

            this->layout()->addWidget(this->_combo);

        };

        void loadTemplate(const RPZAtom::Updates &defaultValues, const AtomSubEditor::LoadingContext &context) override {
            
            AtomSubEditor::loadTemplate(defaultValues, context);

            QSignalBlocker b(this->_combo);
            
            auto data = defaultValues[this->_params.first()];
            auto indexToSelect = this->_combo->findData(data);
            this->_combo->setCurrentIndex(indexToSelect);
            
        }

};