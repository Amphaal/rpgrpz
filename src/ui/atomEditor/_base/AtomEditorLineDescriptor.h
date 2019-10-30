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
            { RPZAtom::Parameter::Text, QT_TR_NOOP("Text") },
            { RPZAtom::Parameter::ShortDescription, QT_TR_NOOP("Short Description") },
            { RPZAtom::Parameter::Description, QT_TR_NOOP("Description") }
        };

        static inline QHash<RPZAtom::Parameter, QString> _valSuffix = {
            { RPZAtom::Parameter::Rotation, "°" },
            { RPZAtom::Parameter::Scale, QStringLiteral(u"x") },
            { RPZAtom::Parameter::AssetRotation, "°" },
            { RPZAtom::Parameter::AssetScale, QStringLiteral(u"x") },
            { RPZAtom::Parameter::PenWidth, QStringLiteral(u"pt") },
            { RPZAtom::Parameter::TextSize, QStringLiteral(u"pt") },
            { RPZAtom::Parameter::BrushPenWidth, QStringLiteral(u"pt") }
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
            { RPZAtom::Parameter::ShortDescription, QStringLiteral(u":/icons/app/tools/text.png") },
            { RPZAtom::Parameter::Description, QStringLiteral(u":/icons/app/tools/text.png") }
        };
};