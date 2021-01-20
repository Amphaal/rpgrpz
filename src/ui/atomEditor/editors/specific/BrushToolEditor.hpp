// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2021 Guillaume Vara <guillaume.vara@gmail.com>

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

#include <QComboBox>

#include "src/ui/atomEditor/_base/AtomSubEditor.h"

class BrushToolEditor : public AtomSubEditor {
    Q_OBJECT

 private:
    static inline QMap<RPZAtom::BrushType, QString> _strBT {
        { RPZAtom::BrushType::Stamp, QT_TR_NOOP("Stamp") },
        { RPZAtom::BrushType::Rectangle, QT_TR_NOOP("Rectangle") },
        { RPZAtom::BrushType::Ovale, QT_TR_NOOP("Ovale") },
        { RPZAtom::BrushType::RoundBrush, QT_TR_NOOP("Round brush") },
        { RPZAtom::BrushType::Cutter, QT_TR_NOOP("Cutter") },
        { RPZAtom::BrushType::Scissors, QT_TR_NOOP("Scissors") }
    };

    static inline QHash<RPZAtom::BrushType, QString> _BTicons {
        { RPZAtom::BrushType::Stamp, QStringLiteral(u":/icons/app/tools/stamp.png") },
        { RPZAtom::BrushType::Rectangle, QStringLiteral(u":/icons/app/tools/rectangle.png") },
        { RPZAtom::BrushType::Ovale, QStringLiteral(u":/icons/app/tools/ovale.png") },
        { RPZAtom::BrushType::RoundBrush, QStringLiteral(u":/icons/app/tools/roundBrush.png") },
        { RPZAtom::BrushType::Cutter, QStringLiteral(u":/icons/app/tools/cutter.png") },
        { RPZAtom::BrushType::Scissors, QStringLiteral(u":/icons/app/tools/scissors.png") }
    };

    QComboBox* _combo = nullptr;

 public:
    BrushToolEditor() : AtomSubEditor({RPZAtom::Parameter::BrushStyle}) {
        this->setVisible(false);

        this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);

        this->_combo = new QComboBox;

        for (auto i = _strBT.constBegin(); i != _strBT.constEnd(); i++) {
            auto tool = i.key();

            auto translatedName = tr(qUtf8Printable(i.value()));
            auto associatedIcon = QIcon(_BTicons.value(tool));

            this->_combo->addItem(associatedIcon, translatedName, (int)tool);
        }

        QObject::connect(
            this->_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [&](int currentIndex) {
                auto out = this->_combo->currentData();
                emit valueConfirmedForPayload({{this->_params.first(), out}});
        });

        this->layout()->addWidget(this->_combo);
    }

    void loadTemplate(const RPZAtom::Updates &defaultValues, const AtomSubEditor::LoadingContext &context) override {
        AtomSubEditor::loadTemplate(defaultValues, context);

        auto data = defaultValues[this->_params.first()];
        auto indexToSelect = this->_combo->findData(data);

        QSignalBlocker b(this->_combo);
        this->_combo->setCurrentIndex(indexToSelect);
        this->_combo->setEnabled(context.mode == AtomSubEditor::EditMode::Template);
    }
};
