#pragma once

#include <QWidget>
#include <QVBoxLayout>

#include "AtomEditorLineDescriptor.h"

#include "src/shared/models/RPZAtom.h"
#include "src/shared/payloads/Payloads.h"

class AtomSubEditor : public QWidget {

    Q_OBJECT

    public:
        enum class EditMode { None, Template, Selection };
        Q_ENUM(EditMode)

        struct LoadingContext {
            AtomSubEditor::EditMode mode;
            int numberOfItems;
        };

        AtomSubEditor(const QList<RPZAtom::Parameter> &parameters, bool supportsBatchEditing = true);
        
        const QList<RPZAtom::Parameter> params();
        virtual void loadTemplate(const RPZAtom::Updates &defaultValues, const AtomSubEditor::LoadingContext &context);

        static bool mustShowBrushPenWidth(const QVariant &brushTypeDefaultValue);

    signals:
        void valueConfirmedForPayload(const RPZAtom::Updates &updates);
        void valueConfirmedForPreview(const RPZAtom::Parameter &parameter, const QVariant &value);

    protected:
        QList<RPZAtom::Parameter> _params;

        AtomEditorLineDescriptor* _descr = nullptr;
        virtual void _handleVisibilityOnLoad(const RPZAtom::Updates &defaultValues);
    
    private:
        bool _supportsBatchEditing = true;
};
inline uint qHash(const AtomSubEditor::EditMode &key, uint seed = 0) {return uint(key) ^ seed;}
