#pragma once

#include <QVBoxLayout>
#include <QWidget>
#include <QGroupBox>

#include <QPushButton>
#include <QLabel>
#include <QSlider>

#include <QtMath>

#include "src/shared/payloads/Payloads.h"
#include "src/shared/models/RPZAtom.h"

#include "base/AtomSubEditor.h"
#include "editors/AtomSliderEditor.h"
#include "editors/NonLinearAtomSliderEditor.hpp"
#include "editors/BrushToolEditor.hpp"
#include "editors/AtomTextEditor.hpp"

#include "src/shared/async-ui/AlterationHandler.h"

class AtomEditor : public QGroupBox {

    Q_OBJECT

    public:
        enum EditMode { None, Template, Selection };

        AtomEditor(QWidget* parent = nullptr);
        void buildEditor(const QVector<const RPZAtom*> &atomsToBuildFrom);
        void resetParams();
    
    signals:
        void requiresPreview(const QVector<RPZAtomId> &RPZAtomIdsToPreview, const AtomParameter &parameter, QVariant &value);
    
    private:
        static inline QHash<EditMode, QString> _strEM {
            { None, "Rien à modifier" },
            { Template, "Modification de modèle" },
            { Selection, "Modification de selection" }
        };

        QVector<const RPZAtom*> _atoms;
        QVector<RPZAtomId> _RPZAtomIds;

        QMap<AtomParameter, AtomSubEditor*> _editorsByParam;
        QList<AtomParameter> _visibleEditors;

        AtomUpdates _findDefaultValuesToBind();
        void _createEditorsFromAtomParameters();

        void _onSubEditorChanged(const AtomParameter &parameterWhoChanged, QVariant &value);
        void _onPreviewRequested(const AtomParameter &parameter, QVariant &value);

        void _emitPayload(AlterationPayload &payload);

        EditMode _currentEditMode = None;
        void _updateEditMode();
};