#include "AtomEditor.h"

AtomEditor::AtomEditor(QWidget* parent) : QWidget(parent) {
    
    this->setLayoutDirection(Qt::LayoutDirection::LeftToRight);
    auto layout = new QVBoxLayout;
    this->setLayout(layout);

    this->_createEditors();
}

void AtomEditor::_createEditors() {
    this->_editorsByParam[RPZAtom::Rotation] = new AtomSliderEditor(RPZAtom::Rotation, 0, 359);
    this->_editorsByParam[RPZAtom::Scale] = new NonLinearAtomSliderEditor(RPZAtom::Scale, 1, 1000);
    this->_editorsByParam[RPZAtom::PenWidth] = new AtomSliderEditor(RPZAtom::PenWidth, 1, 50);
    this->_editorsByParam[RPZAtom::TextSize] = new AtomSliderEditor(RPZAtom::TextSize, 1, 50);

    for(auto editor : this->_editorsByParam) {
        QObject::connect(
            editor, &AtomSubEditor::valueConfirmedForPayload,
            this, &AtomEditor::_onSubEditorChanged
        );
        this->layout()->addWidget(editor);
    }
}

void AtomEditor::_onSubEditorChanged(const RPZAtom::Parameters &parameterWhoChanged, QVariant &value) {
    
    QVector<snowflake_uid> ids;
    for(auto atom : this->_atoms) ids.append(atom->id());
    
    auto payload = MetadataChangedPayload(ids, parameterWhoChanged, value);
    payload.changeSource(AlterationPayload::Source::Local_AtomEditor);
    emit requiresAtomAlteration(payload);

}

QHash<RPZAtom::Parameters, QVariant> AtomEditor::_findDefaultValuesToBind() {
    QHash<RPZAtom::Parameters, QVariant> out;

    if(this->_atoms.count() > 0) {

        auto firstItem = this->_atoms[0];

        //intersect all customizables params
        QSet<RPZAtom::Parameters> paramsToDisplay = firstItem->customizableParams();
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

void AtomEditor::buildEditor(QVector<void*> &atomsToBuildFrom) {
    
    //modify atom list
    this->_atoms.clear();
    for(auto atom : atomsToBuildFrom) this->_atoms.append((RPZAtom*)atom);
    
    //fetch parameter editors to display
    auto toDisplay = this->_findDefaultValuesToBind();

    //load those who need to be displayed
    for(auto i = toDisplay.begin(); i != toDisplay.end(); ++i) {
        this->_editorsByParam[i.key()]->loadTemplate(this->_atoms, i.value());
    }

    //hide the others
    auto toHide = _editorsByParam.keys().toSet().subtract(
        toDisplay.keys().toSet()
    );
    for(auto i : toHide) this->_editorsByParam[i]->setVisible(false);
}
