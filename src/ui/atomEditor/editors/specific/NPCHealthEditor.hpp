#pragma once

#include "src/ui/atomEditor/_base/AtomEditorLineDescriptor.h"
#include <QVBoxLayout>
#include <QSpinBox>

#include "src/shared/models/RPZAtom.h"
#include "src/ui/atomEditor/_base/AtomSubEditor.h"

class NPCHealthEditor : public AtomSubEditor {

    Q_OBJECT

    private:    
        QHash<RPZAtom::Parameter, QSpinBox*> _spins;

    public:
        NPCHealthEditor() : AtomSubEditor({RPZAtom::Parameter::NPCHealth, RPZAtom::Parameter::MinNPCHealth, RPZAtom::Parameter::MinNPCHealth}) { 

            this->setVisible(false);

            this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
            
            this->_spins[RPZAtom::Parameter::MinNPCHealth] = new QSpinBox;
            this->_spins[RPZAtom::Parameter::NPCHealth] = new QSpinBox;
            this->_spins[RPZAtom::Parameter::MaxNPCHealth] = new QSpinBox;

            auto l = (QVBoxLayout*)this->layout();
            auto b = new QHBoxLayout;

            for(auto i = this->_spins.begin(); i != this->_spins.end(); i++) {
                
                QObject::connect(
                    i.value(), QOverload<int>::of(&QSpinBox::valueChanged),
                    [&](int value) {
                        emit valueConfirmedForPayload({{i.key(), value}});
                    }
                );

                b->addWidget(i.value());

            }

            l->addLayout(b);

        };

        void loadTemplate(const RPZAtom::Updates &defaultValues, const AtomSubEditor::EditMode &editMode) override {
            
            AtomSubEditor::loadTemplate(defaultValues, editMode);

            for(auto i = this->_spins.begin(); i != this->_spins.end(); i++) {
                QSignalBlocker b(i.value());
                auto data = defaultValues[i.key()].toInt();
                i.value()->setValue(data);
            }
 
        }

};