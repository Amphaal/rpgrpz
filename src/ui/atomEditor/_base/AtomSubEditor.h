#pragma once

#include <QWidget>
#include <QVBoxLayout>

#include "AtomEditorLineDescriptor.h"

#include "src/shared/models/RPZAtom.h"
#include "src/shared/payloads/Payloads.h"

class AtomSubEditor : public QWidget {

    Q_OBJECT

    public:
        typedef QHash<RPZAtom::Parameter, QVariant> FilteredDefaultValues;
        AtomSubEditor(const QList<RPZAtom::Parameter> &parameters);
        
        const QList<RPZAtom::Parameter> params();
        virtual const AtomSubEditor::FilteredDefaultValues loadTemplate(const RPZAtom::Updates &defaultValues, bool updateMode = false);

        static bool mustShowBrushPenWidth(const QVariant &brushTypeDefaultValue);

    signals:
        void valueConfirmedForPayload(const RPZAtom::Updates &updates);
        void valueConfirmedForPreview(const RPZAtom::Parameter &parameter, const QVariant &value);

    protected:
        QList<RPZAtom::Parameter> _params;
        QWidget* _dataEditor = nullptr;
        void _setAsDataEditor(QWidget *dataEditor);

        AtomEditorLineDescriptor* _descr = nullptr;
        virtual void _handleVisibilityOnLoad(const RPZAtom::Updates &defaultValues);
};