#pragma once

#include "src/ui/atoms/base/AtomSliderEditor.h"

class AtomPenWidthEditor : public AtomSliderEditor {
    
    public:
        AtomPenWidthEditor() : AtomSliderEditor("Taille:", "pt", 1, 5000) { }

        AlterationPayload createPayload() override {
            return MetadataChangedPayload(this->_atomsToSnowflakeList(), RPZAtom::Parameters::PenWidth, this->outputValue());
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