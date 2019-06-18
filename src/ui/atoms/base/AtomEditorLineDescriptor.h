#pragma once

#include <QWidget>
#include <QLabel>
#include <QString>
#include <QHBoxLayout>

#include "src/shared/models/RPZAtom.h"

class AtomEditorLineDescriptor : public QWidget {
    public:
        AtomEditorLineDescriptor(const QString &description, const QString &suffix, QString &iconPath = QString());
        AtomEditorLineDescriptor(const AtomParameter &paramType);
        void updateValue(double value);
        void cannotDisplayValue();
    
    private:
        QString _suffix;
        QLabel* _valLbl = nullptr;

        static inline QHash<AtomParameter, QString> _ParamDescr = {
            { AtomParameter::Rotation, "Rotation" },
            { AtomParameter::Scale, "Taille de l'atome" },
            { AtomParameter::AssetRotation, "Rotation la texture" },
            { AtomParameter::AssetScale, "Taille de la texture" },
            { AtomParameter::PenWidth, "Taille du pinceau" },
            { AtomParameter::TextSize, "Taille du texte" },
            { AtomParameter::BrushPenWidth, "Taille de la brosse" },
            { AtomParameter::BrushStyle, "Outil de terrain" }
        };

        static inline QHash<AtomParameter, QString> _valSuffix = {
            { AtomParameter::Rotation, "°" },
            { AtomParameter::Scale, "x" },
            { AtomParameter::AssetRotation, "°" },
            { AtomParameter::AssetScale, "x" },
            { AtomParameter::PenWidth, "pt" },
            { AtomParameter::TextSize, "pt" },
            { AtomParameter::BrushPenWidth, "pt" },
            { AtomParameter::BrushStyle, "" }
        };

        static inline QHash<AtomParameter, QString> _icons = {
            { AtomParameter::Rotation, ":/icons/app/tools/rotate.png" },
            { AtomParameter::Scale, ":/icons/app/tools/scale.png" },
            { AtomParameter::AssetRotation, ":/icons/app/tools/rotate.png" },
            { AtomParameter::AssetScale, ":/icons/app/tools/scale.png" },
            { AtomParameter::PenWidth, ":/icons/app/tools/pen.png" },
            { AtomParameter::TextSize, ":/icons/app/tools/text.png" },
            { AtomParameter::BrushPenWidth, ":/icons/app/tools/roundBrush.png" },
            { AtomParameter::BrushStyle, ":/icons/app/tabs/config.png" }
        };
};