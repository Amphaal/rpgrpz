#pragma once

#include <QWidget>
#include <QVBoxLayout>

#include "AtomEditorLineDescriptor.h"

#include "src/shared/models/RPZAtom.h"
#include "src/shared/payloads/Payloads.h"

class AtomSubEditor : public QWidget {

    Q_OBJECT

    public:
        AtomSubEditor(const RPZAtom::Parameters &parameter);
        
        RPZAtom::Parameters param();
        void virtual loadTemplate(QVector<RPZAtom*> &atomsToManipulate, QVariant &defaultValue);

    signals:
        void valueConfirmedForPayload(const RPZAtom::Parameters &parameter, QVariant &value);

    protected:
        RPZAtom::Parameters _param;
        QWidget* _dataEditor = nullptr;
        void _setAsDataEditor(QWidget *dataEditor);

        AtomEditorLineDescriptor* _descr = nullptr;
        QVector<RPZAtom*> _atomsToManipulate;
    
    private:
        static inline QHash<RPZAtom::Parameters, QString> _ParamDescr = {
            { RPZAtom::Parameters::Rotation, "Rotation" },
            { RPZAtom::Parameters::Scale, "Taille de l'atome" },
            { RPZAtom::Parameters::PenWidth, "Taille du pinceau" },
            { RPZAtom::Parameters::TextSize, "Taille du texte" }
        };

        static inline QHash<RPZAtom::Parameters, QString> _valSuffix = {
            { RPZAtom::Parameters::Rotation, "°" },
            { RPZAtom::Parameters::Scale, "x" },
            { RPZAtom::Parameters::PenWidth, "pt" },
            { RPZAtom::Parameters::TextSize, "pt" }
        };
};