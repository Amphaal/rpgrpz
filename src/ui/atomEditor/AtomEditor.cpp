// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2021 Guillaume Vara <guillaume.vara@gmail.com>

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

#include "AtomEditor.h"

AtomEditor::AtomEditor(QWidget* parent) : QGroupBox(parent), AlterationInteractor(Payload::Interactor::Local_AtomEditor) {
    auto title = _strEM.value(AtomSubEditor::EditMode::None);
    this->setTitle(tr(qUtf8Printable(title)));
    this->setAlignment(Qt::AlignHCenter);

    this->setLayoutDirection(Qt::LayoutDirection::LeftToRight);
    this->setLayout(new QVBoxLayout);

    // create params editors
    this->_createEditorsFromAtomParameters();

    // message indicating no editors available
    this->_noEditorMsgWidget = new NoEditorMessageWidget;
    this->layout()->addWidget(this->_noEditorMsgWidget);
}

void AtomEditor::buildEditor(const AtomsSelectionDescriptor &atomsSelectionDescr) {
    // modify atom list
    this->_currentSelectionDescr = atomsSelectionDescr;

    // clear editors
    for (auto editor : this->_visibleEditors) editor->setVisible(false);
    this->_visibleEditors.clear();

    // update edit mode
    this->_updateEditMode();

    // fetch default values for adaptated params to context
    auto filteredDefaultValues = this->_findDefaultValuesToBind();

    // find editors to display
    QSet<AtomSubEditor*> toDisplay;
    for (auto const &param : filteredDefaultValues.keys()) {
        auto editor = this->_editorsByParam.value(param);
        if (!editor) continue;
        toDisplay += editor;
    }

    // setup context for loading
    AtomSubEditor::LoadingContext context;
    context.mode = this->_currentEditMode;
    context.numberOfItems = this->_currentSelectionDescr.selectedAtomIds.count();
    context.representedTypes = this->_currentSelectionDescr.representedTypes;

    // load those who need to be displayed
    for (auto editor : toDisplay) {
        // load template, and display them
        editor->loadTemplate(filteredDefaultValues, context);

        // add to the visible editors list
        this->_visibleEditors.append(editor);
    }

    // if no editor is displayed, show a little message
    auto visibility = !this->hasVisibleEditors();
    this->_noEditorMsgWidget->setVisible(visibility);
}

//
//
//

void AtomEditor::resetParams() {
    // reset displayed params
    RPZAtom::Updates changes;
    for (auto editor : this->_visibleEditors) {
        for (auto const &param : editor->params()) {
            changes.insert(
                param,
                RPZAtom::getDefaultValueForParam(param)
            );
        }
    }

    // update inner selection accordingly
    this->_currentSelectionDescr.templateAtom.setMetadata(changes);

    // rebuild
    this->buildEditor(this->_currentSelectionDescr);

    // emit modifications
    this->_emitPayload(changes);
}

void AtomEditor::_addEditor(AtomSubEditor* editor) {
    for (auto &param : editor->params()) {
        this->_editorsByParam.insert(param, editor);
    }

    this->_orderedCreation += editor;
}

void AtomEditor::_createEditorsFromAtomParameters() {
    _addEditor(new TokenSizeEditor);
    _addEditor(new NPCAttitudeEditor);
    _addEditor(new CharacterPickerEditor);

    _addEditor(new BrushToolEditor);
    _addEditor(new AtomSliderEditor(RPZAtom::Parameter::BrushPenWidth, 1, 500));

    _addEditor(new AtomSliderEditor(RPZAtom::Parameter::Rotation, 0, 359));

    CrossEquities defaultEquities({
        { .1, 1 },
        { 1, 500 },
        { 10.0, 1000 }
    });
    _addEditor(new NonLinearAtomSliderEditor(RPZAtom::Parameter::Scale, defaultEquities));

    _addEditor(new AtomSliderEditor(RPZAtom::Parameter::AssetRotation, 0, 359));

    _addEditor(new NonLinearAtomSliderEditor(RPZAtom::Parameter::AssetScale, defaultEquities));

    _addEditor(new AtomSliderEditor(RPZAtom::Parameter::PenWidth, 1, 50));
    _addEditor(new AtomSliderEditor(RPZAtom::Parameter::TextSize, 1, 50));
    _addEditor(new AtomTextEditor(RPZAtom::Parameter::Text));

    _addEditor(new AtomSliderEditor(RPZAtom::Parameter::Opacity, 50, 100));

    _addEditor(new AtomShortTextEditor(RPZAtom::Parameter::EventShortDescription));
    _addEditor(new AtomTextEditor(RPZAtom::Parameter::EventDescription));

    _addEditor(new AtomShortTextEditor(RPZAtom::Parameter::NPCShortName));
    _addEditor(new AtomTextEditor(RPZAtom::Parameter::NPCDescription));
    _addEditor(new NPCHealthEditor);

    _addEditor(new ColorPickerEditor(RPZAtom::Parameter::PenColor));

    // integrate
    this->_integrateEditors();
}

void AtomEditor::_integrateEditors() {
    for (auto editor : this->_orderedCreation) {
        QObject::connect(
            editor, &AtomSubEditor::valueConfirmedForPayload,
            this, &AtomEditor::_emitPayload
        );

        QObject::connect(
            editor, &AtomSubEditor::valueConfirmedForPreview,
            this, &AtomEditor::_onPreviewRequested
        );

        this->layout()->addWidget(editor);
    }
}

void AtomEditor::_onPreviewRequested(const RPZAtom::Parameter &parameter, const QVariant &value) {
    emit requiresPreview(this->_currentSelectionDescr, parameter, value);
}

void AtomEditor::_emitPayload(const RPZAtom::Updates &changesToEmit) {
    // intercept combo change for visibility
    this->_mustShowBrushPenWidthEditor(changesToEmit);

    // if template mode, update template
    if (this->_currentEditMode == AtomSubEditor::EditMode::Template) {
        AtomTemplateChangedPayload payload(changesToEmit);
        AlterationHandler::get()->queueAlteration(this, payload);
    } else {  // else, update selection
        MetadataChangedPayload payload(
            this->_currentSelectionDescr.selectedAtomIds,
            changesToEmit
        );

        AlterationHandler::get()->queueAlteration(this, payload);
    }
}

RPZAtom::Updates AtomEditor::_findDefaultValuesToBind() {
    RPZAtom::Updates out;

    // intersect represented atom types in selection to determine which editors to display
    QSet<RPZAtom::Parameter> paramsToDisplay;
    for (const auto &type : this->_currentSelectionDescr.representedTypes) {
        auto associatedCustomParams = RPZAtom::customizableParams(type);

        if (paramsToDisplay.empty()) {
            paramsToDisplay = associatedCustomParams;
            continue;
        }

        paramsToDisplay = paramsToDisplay.intersect(associatedCustomParams);
        if (!paramsToDisplay.count()) break;
    }

    for (const auto param : paramsToDisplay) {
        out.insert(
            param,
            this->_currentSelectionDescr.templateAtom.metadata(param)
        );
    }

    return out;
}


void AtomEditor::_updateEditMode() {
    // determine edit mode
    auto selectedIdsCount = this->_currentSelectionDescr.selectedAtomIds.count();

    if (selectedIdsCount) {
        this->_currentEditMode = AtomSubEditor::EditMode::Selection;
    } else if (!this->_currentSelectionDescr.templateAtom.isEmpty()) {
        this->_currentEditMode = AtomSubEditor::EditMode::Template;
    } else {
        this->_currentEditMode = AtomSubEditor::EditMode::None;
    }

    // update title
    auto title = tr(qUtf8Printable(_strEM.value(this->_currentEditMode)));

    switch (this->_currentEditMode) {
        case AtomSubEditor::EditMode::Selection:
            title += tr(" (%n element(s))", "", selectedIdsCount);
        break;

        case AtomSubEditor::EditMode::Template:
            title += QStringLiteral(u" [%1]").arg(this->_currentSelectionDescr.templateAtom.toString());
        break;

        default:
        break;
    }

    this->setTitle(title);
}

bool AtomEditor::hasVisibleEditors() {
    return this->_visibleEditors.count();
}

AtomsSelectionDescriptor AtomEditor::currentSelectionDescriptor() {
    return this->_currentSelectionDescr;
}

void AtomEditor::_mustShowBrushPenWidthEditor(const RPZAtom::Updates &updatedValues) {
    // check if param is tool combo
    if (!updatedValues.contains(RPZAtom::Parameter::BrushStyle)) return;

    // check if pen size editor exists
    auto brushPenWidthEditor = this->_editorsByParam.value(RPZAtom::Parameter::BrushPenWidth);
    if (!brushPenWidthEditor) return;

    // set visibility
    auto defaultValue = updatedValues.value(RPZAtom::Parameter::BrushStyle);
    auto mustShow = AtomSubEditor::mustShowBrushPenWidth(defaultValue);
    brushPenWidthEditor->setVisible(mustShow);
}
