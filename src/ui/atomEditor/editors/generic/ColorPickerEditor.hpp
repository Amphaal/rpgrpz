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

#include <QColorDialog>
#include <QPushButton>

#include "src/ui/atomEditor/_base/AtomSubEditor.h"

class ColorPickerEditor : public AtomSubEditor {
 public:
    explicit ColorPickerEditor(const RPZAtom::Parameter &parameter) : AtomSubEditor({parameter}), _colorSelectBtn(new QPushButton(this)) {
            this->layout()->addWidget(this->_colorSelectBtn);

            QObject::connect(
                this->_colorSelectBtn, &QPushButton::pressed,
                this, &ColorPickerEditor::_onColorPickerPushed
            );
        }

 private:
        QPushButton* _colorSelectBtn = nullptr;
        QColor _currentColor;

        void _onColorPickerPushed() {
            auto selectedColor = QColorDialog::getColor(
                this->_currentColor,
                this,
                QObject::tr("Pen color")
            );
            if (!selectedColor.isValid()) return;

            this->_setColorOnPickerButton(selectedColor);

            auto out = QVariant::fromValue(selectedColor);
            emit valueConfirmedForPayload({{this->_params.first(), out}});
        }

        // apply to color picker
        void _setColorOnPickerButton(const QColor &color) {
            this->_currentColor = color;
            this->_colorSelectBtn->setStyleSheet(QStringLiteral(u"QPushButton { background-color: %1; }").arg(this->_currentColor.name()));
        }

        void loadTemplate(const RPZAtom::Updates &defaultValues, const AtomSubEditor::LoadingContext &context) override {
            AtomSubEditor::loadTemplate(defaultValues, context);
            auto castedVal = defaultValues[this->_params.first()].value<QColor>();

            this->_setColorOnPickerButton(castedVal);
        }
};
