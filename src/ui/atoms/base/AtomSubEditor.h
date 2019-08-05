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
        void valueConfirmedForPreview(const AtomParameter &parameter, QVariant &value);

    protected:
        AtomParameter _param;
        QWidget* _dataEditor = nullptr;
        void _setAsDataEditor(QWidget *dataEditor);

        AtomEditorLineDescriptor* _descr = nullptr;
        QVector<RPZAtom*> _atomsToManipulate;
};