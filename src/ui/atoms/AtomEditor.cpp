#include "AtomEditor.h"

AtomEditor::AtomEditor(QWidget* parent) : QGroupBox(_strEM[None], parent) {

    this->setAlignment(Qt::AlignHCenter);

    this->setLayoutDirection(Qt::LayoutDirection::LeftToRight);
    this->setLayout(new QVBoxLayout);

    //custom editors
        
        //brush tool
        this->_burshToolSelector = new BrushToolEditor;
        this->layout()->addWidget(this->_burshToolSelector);
        QObject::connect(
            this->_burshToolSelector, &BrushToolEditor::brushToolChanged,
            this, &AtomEditor::_onBrushToolChange
        );


    //create params editors
    this->_createEditorsFromAtomParameters();
}


void AtomEditor::buildEditor(QVector<RPZAtom*> &atomsToBuildFrom) {
    
    //modify atom list
    this->_atoms = atomsToBuildFrom;
    this->_visibleEditors.clear();

    //update edit mode
    auto editMode = this->_changeEditMode();

    //fetch parameter editors to display
    auto toDisplay = this->_findDefaultValuesToBind();

    //load those who need to be displayed
    for(auto i = toDisplay.begin(); i != toDisplay.end(); ++i) {
        
        //prepare
        auto param = i.key();
        auto editor = this->_editorsByParam[param];

        //load template, and display them
        editor->loadTemplate(this->_atoms, i.value());

        //add to the visible editors list
        this->_visibleEditors.append(param);

    }

    //hide the others
    auto toHide = _editorsByParam.keys().toSet().subtract(
        toDisplay.keys().toSet()
    );
    for(auto i : toHide) this->_editorsByParam[i]->setVisible(false);
}

//
//
//

void AtomEditor::resetParams() {

    //reset displayed params
        QHash<AtomParameter, QVariant> changes;
        for(auto param : this->_visibleEditors) {
            changes.insert(param, QVariant());
        }

        auto payload = MetadataChangedPayload(this->_atomIds(), changes);
        this->_emitPayload(payload);

    //resetBrushTool
    if(this->_burshToolSelector->isVisible()) {
        this->_burshToolSelector->reset();
    }


}

void AtomEditor::_createEditorsFromAtomParameters() {
    this->_editorsByParam[AtomParameter::Rotation] = new AtomSliderEditor(AtomParameter::Rotation, 0, 359);
    this->_editorsByParam[AtomParameter::Scale] = new NonLinearAtomSliderEditor(AtomParameter::Scale, 1, 1000);
    
    this->_editorsByParam[AtomParameter::AssetRotation] = new AtomSliderEditor(AtomParameter::AssetRotation, 0, 359);
    this->_editorsByParam[AtomParameter::AssetScale] = new NonLinearAtomSliderEditor(AtomParameter::AssetScale, 1, 1000);

    this->_editorsByParam[AtomParameter::PenWidth] = new AtomSliderEditor(AtomParameter::PenWidth, 1, 50);
    this->_editorsByParam[AtomParameter::TextSize] = new AtomSliderEditor(AtomParameter::TextSize, 1, 50);

    for(auto editor : this->_editorsByParam) {
        QObject::connect(
            editor, &AtomSubEditor::valueConfirmedForPayload,
            this, &AtomEditor::_onSubEditorChanged
        );
        this->layout()->addWidget(editor);
    }
}

QVector<snowflake_uid> AtomEditor::_atomIds() {
    QVector<snowflake_uid> ids;
    for(auto atom : this->_atoms) ids.append(atom->id());
    return ids;
}

void AtomEditor::_onSubEditorChanged(const AtomParameter &parameterWhoChanged, QVariant &value) {
    auto payload = MetadataChangedPayload(this->_atomIds(), parameterWhoChanged, value);
    this->_emitPayload(payload);
}

 void AtomEditor::_onBrushToolChange(int selectedBrushTool, int brushWidth) {  
     emit requestBurshToolChange(selectedBrushTool, brushWidth);  
 }

void AtomEditor::_emitPayload(AlterationPayload &payload) {
    payload.changeSource(AlterationPayload::Source::Local_AtomEditor);
    emit requiresAtomAlteration(payload);
}

QHash<AtomParameter, QVariant> AtomEditor::_findDefaultValuesToBind() {
    QHash<AtomParameter, QVariant> out;

    if(this->_atoms.count() > 0) {

        auto firstItem = this->_atoms[0];

        //intersect all customizables params
        QSet<AtomParameter> paramsToDisplay = firstItem->customizableParams();
        for(int i = 1; i < this->_atoms.count(); i++) {
            auto currCP = this->_atoms[i]->customizableParams();
            paramsToDisplay = paramsToDisplay.intersect(currCP);
        }

        //find default values to apply
        auto hasMoreThanOneAtomToBind = this->_atoms.count() > 1;
        for(auto param : paramsToDisplay) {
            QVariant val =  hasMoreThanOneAtomToBind ? QVariant() : firstItem->metadata(param);
            out.insert(param, val);
        }

    }

    return out;
}


AtomEditor::EditMode AtomEditor::_changeEditMode() {
    EditMode currentEditMode;
    auto countItems = this->_atoms.count();
    
    if(countItems == 0) {
        currentEditMode  = EditMode::None;
    }
    else if(countItems == 1 && !this->_atoms.at(0)->id()) {
        currentEditMode = EditMode::Template;
    } 
    else {
        currentEditMode = EditMode::Selection;
    }

    //update title
    this->setTitle(_strEM[currentEditMode]);

    //update tool brush selector visibility
    this->_burshToolSelector->setVisible(currentEditMode == Template);

    return currentEditMode;
}