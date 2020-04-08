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

// Any graphical resources available within the source code may 
// use a different license and copyright : please refer to their metadata
// for further details. Graphical resources without explicit references to a
// different license and copyright still refer to this GNU General Public License.

#pragma once

#include "src/ui/atomEditor/_base/AtomEditorLineDescriptor.h"
#include <QVBoxLayout>
#include <QComboBox>

#include "src/shared/models/RPZAtom.h"
#include "src/ui/atomEditor/_base/AtomSubEditor.h"

class TokenSizeEditor : public AtomSubEditor {

    Q_OBJECT

    private:    
        static inline QMap<RPZAtom::TokenSize, QString> _strTokenSize {
            { RPZAtom::TokenSize::Normal, QT_TR_NOOP("Normal") },
            { RPZAtom::TokenSize::Big, QT_TR_NOOP("Big") },
            { RPZAtom::TokenSize::VeryBig, QT_TR_NOOP("Very Big") },
        };

        static inline QHash<RPZAtom::TokenSize, QString> _tokenSizeIcons {
            { RPZAtom::TokenSize::Normal, QStringLiteral(u":/icons/app/tokenSize/medium.png") },
            { RPZAtom::TokenSize::Big, QStringLiteral(u":/icons/app/tokenSize/large.png") },
            { RPZAtom::TokenSize::VeryBig, QStringLiteral(u":/icons/app/tokenSize/xxl.png") },
        };

        QComboBox* _combo = nullptr;

    public:
        TokenSizeEditor() : AtomSubEditor({RPZAtom::Parameter::TokenSize}) { 

            this->setVisible(false);

            this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
            
            this->_combo = new QComboBox;
            
            for(auto i = _strTokenSize.constBegin(); i != _strTokenSize.constEnd(); i++) {

                auto tool = i.key();
                
                auto translatedName = tr(qUtf8Printable(i.value()));
                auto associatedIcon = QIcon(_tokenSizeIcons.value(tool));

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