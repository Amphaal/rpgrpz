#include "AtomEditor.h"

AtomEditorLineDescriptor::AtomEditorLineDescriptor(const QString &description, const QString &suffix, QWidget* parent) : QWidget(parent), 
    _suffix(suffix),
    _valLbl(new QLabel(this)) {
    
    auto hLayout = new QHBoxLayout;
    this->setLayout(hLayout);
    
    hLayout->addWidget(new QLabel(description));
    hLayout->addStretch(0);
    hLayout->addWidget(this->_valLbl);
}

void AtomEditorLineDescriptor::updateValue(double value) {
    this->_valLbl->setText(QString::number(value) + this->_suffix);
}

void AtomEditorLineDescriptor::cannotDisplayValue() {
    this->_valLbl->setText("val. mult.");
}

AtomEditor::AtomEditor(QWidget* parent) : QGroupBox("Paramétrage", parent) {
    
    //prepare
    this->setAlignment(Qt::AlignHCenter);
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
    this->setLayout(new QVBoxLayout);

    //descriptors
    this->_rotateDescr = new AtomEditorLineDescriptor("Rotation:", "°");
    this->_scaleDescr = new AtomEditorLineDescriptor("Taille:", "x");

    //rotate slider 
    this->_rotateSlider = new QSlider(Qt::Orientation::Horizontal);
    this->_rotateSlider->setMinimum(0);
    this->_rotateSlider->setMaximum(359);
    QObject::connect(
        this->_rotateSlider, &QSlider::valueChanged,
        this, &AtomEditor::_onRotateChanging
    );
    QObject::connect(
        this->_rotateSlider, &QSlider::sliderReleased,
        this, &AtomEditor::_onRotateChanged
    );

    //scale slider
    this->_scaleSlider = new QSlider(Qt::Orientation::Horizontal);
    this->_scaleSlider->setMinimum(_minScaleSlider);
    this->_scaleSlider->setMaximum(_maxScaleSlider);
    QObject::connect(
        this->_scaleSlider, &QSlider::valueChanged,
        this, &AtomEditor::_onScaleChanging
    );
    QObject::connect(
        this->_scaleSlider, &QSlider::sliderReleased,
        this, &AtomEditor::_onScaleChanged
    );
}

void AtomEditor::buildEditor(QVector<void*> &atoms) {
    
    //populate
    this->_atoms.clear();
    for(auto atom : atoms) this->_atoms.append((RPZAtom*)atom);

    //if no selection
    if(!atoms.count()) {
        this->_destroyEditor();
        return;
    } 
    
    //if no items
    if(!this->layout()->count()) {

        //rotate
        this->layout()->addWidget(this->_rotateDescr);
        this->layout()->addWidget(this->_rotateSlider);

        //scale
        this->layout()->addWidget(this->_scaleDescr);
        this->layout()->addWidget(this->_scaleSlider);

    }

    //define default values
    RPZAtomMetadata mdata = atoms.count() == 1 ? this->_atoms[0]->metadata() : RPZAtomMetadata();
    auto defaultRotation = mdata.rotation();
    auto defaultScale = mdata.scale();

    //depending on number of items selected...
    if(atoms.count() > 1) {
        this->_rotateDescr->cannotDisplayValue();
        this->_scaleDescr->cannotDisplayValue();
    } else {
        this->_rotateDescr->updateValue(defaultRotation);
        this->_scaleDescr->updateValue(defaultScale);
    }
    
    this->_rotateSlider->blockSignals(true);
        this->_rotateSlider->setValue(defaultRotation);
    this->_rotateSlider->blockSignals(false);
    
    this->_scaleSlider->blockSignals(true);
        this->_scaleSlider->setValue(this->_fromScaleValue(defaultScale));
    this->_scaleSlider->blockSignals(false);

}

//
// Helpers
//

//https://www.wolframalpha.com/input/?i=fit+exponential++%5B(1,+0.1),+(500,+1),+(1000,+10)%5D
double AtomEditor::_toScaleValue(int sliderVal) {
    auto eqResult = (double).0999821 * qExp((double)0.00460535 * sliderVal);
    return round(eqResult * 100) / 100;
}

//https://www.wolframalpha.com/input/?i=fit+log+%5B(.1,+1),+(1,+500),+(10,+1000)%5D
int AtomEditor::_fromScaleValue(double scaled) {
    auto eqResult = (double)216.93 * qLn((double)10.0385 * scaled);
    auto roundedResult = round(eqResult * 100) / 100;
    return roundedResult < 1 ? 1 : roundedResult;
}

QVector<snowflake_uid> AtomEditor::_atomsToSnowflakeList() {
    QVector<snowflake_uid> out;
    for(auto atom : this->_atoms) out.append(atom->id());
    return out;
}

void AtomEditor::_destroyEditor() {
    while (auto item = this->layout()->takeAt(0)) {
        this->layout()->removeItem(item);
    }
}

//
// Event handlers
//

void AtomEditor::_onScaleChanged() {
    auto scaleVal = this->_toScaleValue(this->_scaleSlider->value());
    auto payload = ScaledPayload(this->_atomsToSnowflakeList(), scaleVal);
    emit requiresAtomAlteration(payload);
}

void AtomEditor::_onRotateChanged() {
    auto sliderVal = this->_rotateSlider->value();
    auto payload = RotatedPayload(this->_atomsToSnowflakeList(), sliderVal);
    emit requiresAtomAlteration(payload);
}


void AtomEditor::_onScaleChanging(int sliderVal) {
    
    auto valueAsScale = this->_toScaleValue(sliderVal);

    this->_scaleDescr->updateValue(valueAsScale);

    for(auto atom : this->_atoms) {
        atom->graphicsItem()->setScale(valueAsScale);
    }
}

void AtomEditor::_onRotateChanging(int sliderVal) {

    this->_rotateDescr->updateValue(sliderVal);

    for(auto atom : this->_atoms) {
        atom->graphicsItem()->setRotation(sliderVal);
    }
}