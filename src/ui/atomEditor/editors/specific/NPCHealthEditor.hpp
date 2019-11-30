#pragma once

#include "src/ui/atomEditor/_base/AtomEditorLineDescriptor.h"
#include <QVBoxLayout>
#include <QSpinBox>

#include "src/shared/models/RPZAtom.h"
#include "src/ui/atomEditor/_base/AtomSubEditor.h"
#include "src/ui/sheets/components/SimpleGaugeEditor.hpp"

class NPCHealthEditor : public AtomSubEditor {

    Q_OBJECT

    private:    
        SimpleGaugeEditor* _gEditor = nullptr;
        QPushButton* _validateButton = nullptr;

    public:
        NPCHealthEditor() : AtomSubEditor({RPZAtom::Parameter::NPCHealth, RPZAtom::Parameter::MinNPCHealth, RPZAtom::Parameter::MinNPCHealth}) { 
            
            this->_gEditor = new SimpleGaugeEditor;
            this->_gEditor->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

            this->_validateButton = new QPushButton(this);
            this->_validateButton->setText(tr("Confirm modification"));

            QObject::connect(
                this->_validateButton, &QPushButton::pressed,
                [=]() {
                    
                    auto values = this->_gEditor->toMinimalist();
                    
                    RPZAtom::Updates updates {
                        { RPZAtom::Parameter::NPCHealth, values.current },
                        { RPZAtom::Parameter::MinNPCHealth, values.min },
                        { RPZAtom::Parameter::MaxNPCHealth, values.max }
                    };

                    emit valueConfirmedForPayload(updates);

                }
            );

            this->setVisible(false);
            this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
            
            this->_mainLayout->addWidget(this->_gEditor, 0, Qt::AlignCenter);
            this->_mainLayout->addWidget(this->_validateButton);

        };

        void loadTemplate(const RPZAtom::Updates &defaultValues, const AtomSubEditor::LoadingContext &context) override {

            AtomSubEditor::loadTemplate(defaultValues, context);

            this->_gEditor->fillValues({
                defaultValues.value(RPZAtom::Parameter::NPCHealth).toInt(),
                defaultValues.value(RPZAtom::Parameter::MinNPCHealth).toInt(),
                defaultValues.value(RPZAtom::Parameter::MaxNPCHealth).toInt(),
            });
 
        }

};