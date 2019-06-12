#pragma once

#include "src/ui/atoms/base/AtomSliderEditor.h"

class AtomRotationEditor : public AtomSliderEditor {
    
    public:
        AtomRotationEditor() : AtomSliderEditor("Rotation:", "°", 0, 359) { }

        AlterationPayload createPayload() override {
            return MetadataChangedPayload(this->_atomsToSnowflakeList(), RPZAtom::Parameters::Rotation, this->outputValue());
        }

    private:
        double atomValue(RPZAtom &atom) override {
            return atom.rotation();
        }

        int atomValueToSliderValue(RPZAtom &atom) override {
            return this->atomValue(atom);
        }

        void _updateGraphicsItem(QGraphicsItem* giToUpdate, double value) override {
            giToUpdate->setRotation(value);
        }
};