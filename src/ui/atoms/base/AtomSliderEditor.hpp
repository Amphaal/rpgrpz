#pragma once

#include <QWidget>
#include <QSlider>

#include "AtomEditorLineDescriptor.h"

#include "src/shared/models/entities/RPZAtom.h"
#include "src/ui/atoms/AtomEditor.h"

#include "src/shared/models/payloads/base/AlterationPayload.hpp"

class AtomSliderEditor : public QWidget {
    public:
        AtomSliderEditor(const QString &descr, const QString &suffix, AtomEditor* parent) :
        _parent(parent),
        _slider(new QSlider(Qt::Orientation::Horizontal)),
        _descr(new AtomEditorLineDescriptor(descr, suffix)) { 

            QObject::connect(
                this->_slider, &QSlider::valueChanged,
                this, &AtomSliderEditor::_onSliderChanging
            );
            QObject::connect(
                this->_slider, &QSlider::sliderReleased,
                this, &AtomSliderEditor::_onSliderChanged
            );
            
        }

        void loadAtomsAsTemplate(QVector<RPZAtom*> &atoms) {
            
            this->_atoms = atoms;
            
            auto atomTemplate = this->atomTemplate();
            auto multipleAtoms = this->_atoms.count() > 1;
            
            if(multipleAtoms) {
                this->_descr->cannotDisplayValue();
            } else {
                this->_descr->updateValue(this->atomValue(atomTemplate));
            }
            
            this->_slider->blockSignals(true); 
                this->_slider->setValue(this->atomValueToSliderValue(atomTemplate));
            this->_slider->blockSignals(true);

        }

    protected:
        QSlider* _slider = nullptr;
        AtomEditorLineDescriptor* _descr = nullptr;
        QVector<RPZAtom*> _atoms;

        void _onSliderChanging(int sliderVal) {
            
            auto output = this->outputValue();
            this->_descr->updateValue(output);
            
            for(auto atom : this->_atoms) {
                this->_updateGraphicsItem(atom->graphicsItem(), output);
            }
        };

        void _onSliderChanged() {
            auto payload = this->_createPayload(
                this->_atomsToSnowflakeList(),
                this->outputValue()
            );
            this->_parent->signalPayload(payload);
        };

        virtual double outputValue() {
            return this->_slider->value();
        }
        virtual double atomValue(RPZAtom &atom) = 0;
        virtual int atomValueToSliderValue(RPZAtom &atom) = 0;
        virtual AlterationPayload _createPayload(QVector<snowflake_uid> &toAlter, double newValue) = 0;
        virtual void _updateGraphicsItem(QGraphicsItem* giToUpdate, double value) = 0;

    private:
        AtomEditor* _parent = nullptr;
        RPZAtom atomTemplate() {
            return this->_atoms.count() == 1 ? *this->_atoms[0] : RPZAtom();
        }

        QVector<snowflake_uid> _atomsToSnowflakeList() {
            QVector<snowflake_uid> out;
            for(auto atom : this->_atoms) out.append(atom->id());
            return out;
        }
};