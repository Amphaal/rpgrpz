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

#include <QWidget>
#include <QLabel>
#include <QString>
#include <QHBoxLayout>

#include "src/shared/models/RPZAtom.h"

class AtomEditorLineDescriptor : public QWidget {

    Q_OBJECT

    public:
        AtomEditorLineDescriptor(const RPZAtom::Parameter &paramType, bool supportsBatchEditing);
        void updateValue(double value);
        void cannotDisplayValue();
    
    private:
        AtomEditorLineDescriptor(const QString &untranslatedDescription, const QString &suffix, const QString &iconPath = QString());

        QString _suffix;
        QLabel* _valLbl = nullptr;
        bool _supportsBatchEditing = true;

        static inline QHash<RPZAtom::Parameter, QString> _paramDescr = {
            { RPZAtom::Parameter::Rotation, QT_TR_NOOP("Rotation") },
            { RPZAtom::Parameter::Scale, QT_TR_NOOP("Atom size") },
            { RPZAtom::Parameter::AssetRotation, QT_TR_NOOP("Texture rotation") },
            { RPZAtom::Parameter::AssetScale, QT_TR_NOOP("Texture size") },
            { RPZAtom::Parameter::PenWidth, QT_TR_NOOP("Pen size") },
            { RPZAtom::Parameter::TextSize, QT_TR_NOOP("Text size") },
            { RPZAtom::Parameter::BrushPenWidth, QT_TR_NOOP("Brush width") },
            { RPZAtom::Parameter::BrushStyle, QT_TR_NOOP("Brush tool") },
            { RPZAtom::Parameter::Text, QT_TR_NOOP("Text") },
            { RPZAtom::Parameter::EventShortDescription, QT_TR_NOOP("Short description") },
            { RPZAtom::Parameter::EventDescription, QT_TR_NOOP("GM Notes") },
            { RPZAtom::Parameter::NPCAttitude, QT_TR_NOOP("Attitude") },
            { RPZAtom::Parameter::NPCHealth, QT_TR_NOOP("Health") },
            { RPZAtom::Parameter::NPCShortName, QT_TR_NOOP("Short name") },
            { RPZAtom::Parameter::NPCDescription, QT_TR_NOOP("GM Notes") },
            { RPZAtom::Parameter::CharacterId, QT_TR_NOOP("Associated character") },
            { RPZAtom::Parameter::Opacity, QT_TR_NOOP("Opacity") }
        };

        static inline QHash<RPZAtom::Parameter, QString> _valSuffix = {
            { RPZAtom::Parameter::Rotation, "°" },
            { RPZAtom::Parameter::Scale, QStringLiteral(u"x") },
            { RPZAtom::Parameter::AssetRotation, "°" },
            { RPZAtom::Parameter::AssetScale, QStringLiteral(u"x") },
            { RPZAtom::Parameter::PenWidth, QStringLiteral(u"pts") },
            { RPZAtom::Parameter::TextSize, QStringLiteral(u"pts") },
            { RPZAtom::Parameter::BrushPenWidth, QStringLiteral(u"pts") },
            { RPZAtom::Parameter::Opacity, QStringLiteral(u"%") }
        };

        static inline QHash<RPZAtom::Parameter, QString> _icons = {
            { RPZAtom::Parameter::Rotation, QStringLiteral(u":/icons/app/tools/rotate.png") },
            { RPZAtom::Parameter::Scale, QStringLiteral(u":/icons/app/tools/scale.png") },
            { RPZAtom::Parameter::AssetRotation, QStringLiteral(u":/icons/app/tools/rotate.png") },
            { RPZAtom::Parameter::AssetScale, QStringLiteral(u":/icons/app/tools/scale.png") },
            { RPZAtom::Parameter::PenWidth, QStringLiteral(u":/icons/app/tools/pen.png") },
            { RPZAtom::Parameter::TextSize, QStringLiteral(u":/icons/app/tools/text.png") },
            { RPZAtom::Parameter::BrushPenWidth, QStringLiteral(u":/icons/app/tools/roundBrush.png") },
            { RPZAtom::Parameter::BrushStyle, QStringLiteral(u":/icons/app/tabs/config.png") },
            { RPZAtom::Parameter::Text, QStringLiteral(u":/icons/app/tools/text.png") },
            { RPZAtom::Parameter::EventShortDescription, QStringLiteral(u":/icons/app/tools/text.png") },
            { RPZAtom::Parameter::EventDescription, QStringLiteral(u":/icons/app/tools/text.png") },
            { RPZAtom::Parameter::NPCAttitude, QStringLiteral(u":/icons/app/tools/behavior.png") },
            { RPZAtom::Parameter::NPCHealth, QStringLiteral(u":/icons/app/tools/health.png") },
            { RPZAtom::Parameter::NPCShortName, QStringLiteral(u":/icons/app/tools/text.png") },
            { RPZAtom::Parameter::NPCDescription, QStringLiteral(u":/icons/app/tools/text.png") },
            { RPZAtom::Parameter::CharacterId, QStringLiteral(u":/icons/app/connectivity/cloak.png") },
            { RPZAtom::Parameter::Opacity, QStringLiteral(u":/icons/app/tools/opacity.png") }
        };
};