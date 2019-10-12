#pragma once

#include <QWidget>
#include <QLabel>
#include <QString>
#include <QHBoxLayout>

#include "src/shared/models/RPZAtom.h"

class AtomEditorLineDescriptor : public QWidget {
    public:
        AtomEditorLineDescriptor(const QString &untranslatedDescription, const QString &suffix, const QString &iconPath = QString());
        AtomEditorLineDescriptor(const AtomParameter &paramType);
        void updateValue(double value);
        void cannotDisplayValue();
    
    private:
        QString _suffix;
        QLabel* _valLbl = nullptr;

        static inline QHash<AtomParameter, QString> _paramDescr = {
            { AtomParameter::Rotation, QT_TR_NOOP("Rotation") },
            { AtomParameter::Scale, QT_TR_NOOP("Atom size") },
            { AtomParameter::AssetRotation, QT_TR_NOOP("Texture rotation") },
            { AtomParameter::AssetScale, QT_TR_NOOP("Texture size") },
            { AtomParameter::PenWidth, QT_TR_NOOP("Pen size") },
            { AtomParameter::TextSize, QT_TR_NOOP("Text size") },
            { AtomParameter::BrushPenWidth, QT_TR_NOOP("Brush width") },
            { AtomParameter::BrushStyle, QT_TR_NOOP("Brush tool") },
            { AtomParameter::Text, QT_TR_NOOP("Text") }
        };

        static inline QHash<AtomParameter, QString> _valSuffix = {
            { AtomParameter::Rotation, "°" },
            { AtomParameter::Scale, "x" },
            { AtomParameter::AssetRotation, "°" },
            { AtomParameter::AssetScale, "x" },
            { AtomParameter::PenWidth, "pt" },
            { AtomParameter::TextSize, "pt" },
            { AtomParameter::BrushPenWidth, "pt" },
            { AtomParameter::BrushStyle, "" },
            { AtomParameter::Text, "" }
        };

        static inline QHash<AtomParameter, QString> _icons = {
            { AtomParameter::Rotation, ":/icons/app/tools/rotate.png" },
            { AtomParameter::Scale, ":/icons/app/tools/scale.png" },
            { AtomParameter::AssetRotation, ":/icons/app/tools/rotate.png" },
            { AtomParameter::AssetScale, ":/icons/app/tools/scale.png" },
            { AtomParameter::PenWidth, ":/icons/app/tools/pen.png" },
            { AtomParameter::TextSize, ":/icons/app/tools/text.png" },
            { AtomParameter::BrushPenWidth, ":/icons/app/tools/roundBrush.png" },
            { AtomParameter::BrushStyle, ":/icons/app/tabs/config.png" },
            { AtomParameter::Text, ":/icons/app/tools/text.png" }
        };
};