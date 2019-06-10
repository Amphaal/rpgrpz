#pragma once

#include "src/ui/atoms/base/AtomSliderEditor.hpp"

class AtomRotationEditor : public AtomSliderEditor {
    
    public:
        AtomRotationEditor(AtomEditor* parent) : AtomSliderEditor("Rotation:", "°", parent) {
            this->_slider->setMinimum(0);
            this->_slider->setMaximum(359);
        }
    
    private:
        double atomValue(RPZAtom &atom) override {
            return atom.rotation();
        }

        int atomValueToSliderValue(RPZAtom &atom) override {
            return this->atomValue(atom);
        }

        AlterationPayload _createPayload(QVector<snowflake_uid> &toAlter, double newValue) override {
            return RotatedPayload(toAlter, newValue);
        }

        void _updateGraphicsItem(QGraphicsItem* giToUpdate, double value) override {
            giToUpdate->setRotation(value);
        }
};