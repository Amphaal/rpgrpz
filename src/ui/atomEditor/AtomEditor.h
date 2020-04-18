// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical or audio resources available within the source code may
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GPL.

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

#include "src/ui/atomEditor/_base/NoEditorMessageWidget.hpp"
#include "src/ui/atomEditor/_base/AtomSubEditor.h"

#include "src/ui/atomEditor/editors/generic/AtomSliderEditor.hpp"
#include "src/ui/atomEditor/editors/generic/NonLinearAtomSliderEditor.hpp"
#include "src/ui/atomEditor/editors/generic/AtomTextEditor.hpp"
#include "src/ui/atomEditor/editors/generic/AtomShortTextEditor.hpp"
#include "src/ui/atomEditor/editors/generic/ColorPickerEditor.hpp"

#include "src/ui/atomEditor/editors/specific/BrushToolEditor.hpp"
#include "src/ui/atomEditor/editors/specific/CharacterPickerEditor.hpp"
#include "src/ui/atomEditor/editors/specific/NPCAttitudeEditor.hpp"
#include "src/ui/atomEditor/editors/specific/TokenSizeEditor.hpp"
#include "src/ui/atomEditor/editors/specific/NPCHealthEditor.hpp"

#include "src/shared/async-ui/AlterationInteractor.hpp"
#include "src/shared/hints/HintThread.hpp"

class AtomEditor : public QGroupBox, public AlterationInteractor {

    Q_OBJECT

 public:
        AtomEditor(QWidget* parent = nullptr);
        void buildEditor(const AtomsSelectionDescriptor &atomsSelectionDescr);
        void resetParams();
        bool hasVisibleEditors();
        AtomsSelectionDescriptor currentSelectionDescriptor();
    
    signals:
        void requiresPreview(const AtomsSelectionDescriptor &selectionDescriptor, const RPZAtom::Parameter &parameter, const QVariant &value);

 private:
        static inline QHash<AtomSubEditor::EditMode, QString> _strEM {
            { AtomSubEditor::EditMode::None, QT_TR_NOOP("Nothing to modify") },
            { AtomSubEditor::EditMode::Template, QT_TR_NOOP("Template modification") },
            { AtomSubEditor::EditMode::Selection, QT_TR_NOOP("Selection modification") }
        };

        AtomsSelectionDescriptor _currentSelectionDescr;

        QMap<RPZAtom::Parameter, AtomSubEditor*> _editorsByParam;
        QVector<AtomSubEditor*> _orderedCreation;
        void _addEditor(AtomSubEditor* editor);
        void _integrateEditors();

        NoEditorMessageWidget* _noEditorMsgWidget = nullptr;

        QList<AtomSubEditor*> _visibleEditors;

        RPZAtom::Updates _findDefaultValuesToBind();
        void _createEditorsFromAtomParameters();

        void _onPreviewRequested(const RPZAtom::Parameter &parameter, const QVariant &value);
        void _emitPayload(const RPZAtom::Updates &changesToEmit);

        AtomSubEditor::EditMode _currentEditMode = AtomSubEditor::EditMode::None;
        void _updateEditMode();

        void _mustShowBrushPenWidthEditor(const RPZAtom::Updates &updatedValues);
};
