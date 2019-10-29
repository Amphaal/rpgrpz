#pragma once

#include <QWidget>
#include <QVBoxLayout>

#include "AtomEditorLineDescriptor.h"

#include "src/shared/models/RPZAtom.h"
#include "src/shared/payloads/Payloads.h"

class AtomSubEditor : public QWidget {

    Q_OBJECT

    public:
        AtomSubEditor(const RPZAtom::Parameter &parameter);
        
        RPZAtom::Parameter param();
        virtual QVariant loadTemplate(const RPZAtom::Updates &defaultValues, bool updateMode = false);

        static bool mustShowBrushPenWidth(const QVariant &brushTypeDefaultValue);

    signals:
        void valueConfirmedForPayload(const RPZAtom::Parameter &parameter, const QVariant &value);
        void valueConfirmedForPreview(const RPZAtom::Parameter &parameter, const QVariant &value);

    protected:
        RPZAtom::Parameter _param;
        QWidget* _dataEditor = nullptr;
        void _setAsDataEditor(QWidget *dataEditor);

        AtomEditorLineDescriptor* _descr = nullptr;
        virtual void _handleVisibilityOnLoad(const RPZAtom::Updates &defaultValues);
};