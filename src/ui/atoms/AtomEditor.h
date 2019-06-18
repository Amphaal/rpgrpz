#pragma once

#include <QVBoxLayout>
#include <QWidget>
#include <QGroupBox>

#include <QPushButton>
#include <QLabel>
#include <QSlider>

#include <QtMath>

#include "src/shared/payloads/Payloads.h"

#include "base/AtomSubEditor.h"
#include "editors/AtomSliderEditor.h"
#include "editors/NonLinearAtomSliderEditor.hpp"
#include "editors/BrushToolEditor.hpp"

class AtomEditor : public QGroupBox {

    Q_OBJECT

    public:
        enum EditMode { None, Template, Selection };

        AtomEditor(QWidget* parent = nullptr);
        void buildEditor(QVector<RPZAtom*> &atomsToBuildFrom);
        void resetParams();
    
    signals:
        void requiresAtomAlteration(QVariantHash &payload);
    
    private:

        static inline QHash<EditMode, QString> _strEM {
            { None, "Rien à modifier" },
            { Template, "Modification de modèle" },
            { Selection, "Modification de selection" }
        };

        QVector<RPZAtom*> _atoms;
        QVector<snowflake_uid> _atomIds();

        QMap<AtomParameter, AtomSubEditor*> _editorsByParam;
        QList<AtomParameter> _visibleEditors;

        QHash<AtomParameter, QVariant> _findDefaultValuesToBind();
        void _createEditorsFromAtomParameters();

        void _onSubEditorChanged(const AtomParameter &parameterWhoChanged, QVariant &value);
        void _emitPayload(AlterationPayload &payload);
        EditMode _changeEditMode();
};