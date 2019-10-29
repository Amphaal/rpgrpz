#pragma once

#include <QWidget>
#include <QLabel>
#include <QString>
#include <QHBoxLayout>

#include "src/shared/models/RPZAtom.h"

class AtomEditorLineDescriptor : public QWidget {

    Q_OBJECT

    public:
        AtomEditorLineDescriptor(const QString &untranslatedDescription, const QString &suffix, const QString &iconPath = QString());
        AtomEditorLineDescriptor(const RPZAtom::Parameter &paramType);
        void updateValue(double value);
        void cannotDisplayValue();
    
    private:
        QString _suffix;
        QLabel* _valLbl = nullptr;

        static inline QHash<RPZAtom::Parameter, QString> _paramDescr = {
            { RPZAtom::Parameter::Rotation, QT_TR_NOOP("Rotation") },
            { RPZAtom::Parameter::Scale, QT_TR_NOOP("Atom size") },
            { RPZAtom::Parameter::AssetRotation, QT_TR_NOOP("Texture rotation") },
            { RPZAtom::Parameter::AssetScale, QT_TR_NOOP("Texture size") },
            { RPZAtom::Parameter::PenWidth, QT_TR_NOOP("Pen size") },
            { RPZAtom::Parameter::TextSize, QT_TR_NOOP("Text size") },
            { RPZAtom::Parameter::BrushPenWidth, QT_TR_NOOP("Brush width") },
            { RPZAtom::Parameter::BrushStyle, QT_TR_NOOP("Brush tool") },
            { RPZAtom::Parameter::Text, QT_TR_NOOP("Text") }
        };

        static inline QHash<RPZAtom::Parameter, QString> _valSuffix = {
            { RPZAtom::Parameter::Rotation, "°" },
            { RPZAtom::Parameter::Scale, "x" },
            { RPZAtom::Parameter::AssetRotation, "°" },
            { RPZAtom::Parameter::AssetScale, "x" },
            { RPZAtom::Parameter::PenWidth, "pt" },
            { RPZAtom::Parameter::TextSize, "pt" },
            { RPZAtom::Parameter::BrushPenWidth, "pt" },
            { RPZAtom::Parameter::BrushStyle, "" },
            { RPZAtom::Parameter::Text, "" }
        };

        static inline QHash<RPZAtom::Parameter, QString> _icons = {
            { RPZAtom::Parameter::Rotation, ":/icons/app/tools/rotate.png" },
            { RPZAtom::Parameter::Scale, ":/icons/app/tools/scale.png" },
            { RPZAtom::Parameter::AssetRotation, ":/icons/app/tools/rotate.png" },
            { RPZAtom::Parameter::AssetScale, ":/icons/app/tools/scale.png" },
            { RPZAtom::Parameter::PenWidth, ":/icons/app/tools/pen.png" },
            { RPZAtom::Parameter::TextSize, ":/icons/app/tools/text.png" },
            { RPZAtom::Parameter::BrushPenWidth, ":/icons/app/tools/roundBrush.png" },
            { RPZAtom::Parameter::BrushStyle, ":/icons/app/tabs/config.png" },
            { RPZAtom::Parameter::Text, ":/icons/app/tools/text.png" }
        };
};