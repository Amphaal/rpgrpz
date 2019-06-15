#pragma once

#include <QWidget>
#include <QVBoxLayout>

#include "AtomEditorLineDescriptor.h"

#include "src/shared/models/RPZAtom.h"
#include "src/shared/payloads/Payloads.h"

class AtomSubEditor : public QWidget {

    Q_OBJECT

    public:
        AtomSubEditor(const AtomParameter &parameter);
        
        AtomParameter param();
        void virtual loadTemplate(QVector<RPZAtom*> &atomsToManipulate, QVariant &defaultValue);

    signals:
        void valueConfirmedForPayload(const AtomParameter &parameter, QVariant &value);

    protected:
        AtomParameter _param;
        QWidget* _dataEditor = nullptr;
        void _setAsDataEditor(QWidget *dataEditor);

        AtomEditorLineDescriptor* _descr = nullptr;
        QVector<RPZAtom*> _atomsToManipulate;
    
    private:
        static inline QHash<AtomParameter, QString> _ParamDescr = {
            { AtomParameter::Rotation, "Rotation" },
            { AtomParameter::Scale, "Taille de l'atome" },
            { AtomParameter::AssetRotation, "Rotation la texture" },
            { AtomParameter::AssetScale, "Taille de la texture" },
            { AtomParameter::PenWidth, "Taille du pinceau" },
            { AtomParameter::TextSize, "Taille du texte" }
        };

        static inline QHash<AtomParameter, QString> _valSuffix = {
            { AtomParameter::Rotation, "°" },
            { AtomParameter::Scale, "x" },
            { AtomParameter::AssetRotation, "°" },
            { AtomParameter::AssetScale, "x" },
            { AtomParameter::PenWidth, "pt" },
            { AtomParameter::TextSize, "pt" }
        };
};