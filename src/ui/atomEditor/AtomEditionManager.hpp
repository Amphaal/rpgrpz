#pragma once

#include "AtomEditor.h"

class AtomEditionManager : public QWidget {

    Q_OBJECT

    private:
        AtomEditor* _editor = nullptr;
        QPushButton* _resetButton = nullptr;
        AtomsStorage* _storage = nullptr;

        void _handleSubjectChange(const AtomsSelectionDescriptor &atomsSelectDescriptor) {
            this->_resetButton->setEnabled(false);
            this->_editor->buildEditor(atomsSelectDescriptor);
            this->_resetButton->setEnabled(
                this->_editor->hasVisibleEditors()
            );
        }

    public:
        AtomEditionManager(AtomsStorage* storage, QWidget *parent = nullptr) : QWidget(parent), 
            _storage(storage), 
            _editor(new AtomEditor), 
            _resetButton(new QPushButton) {
            
            this->_resetButton->setText(tr("Reset displayed parameters"));

            auto layout = new QVBoxLayout;
            this->setLayout(layout);
            
            layout->addWidget(this->_editor, 1);
            layout->addWidget(this->_resetButton);


            QObject::connect(
                AlterationHandler::get(), &AlterationHandler::requiresPayloadHandling,
                this, &AtomEditionManager::_handleAlterationRequest
            );

            QObject::connect(
                this->_resetButton, &QPushButton::pressed,
                this->_editor, &AtomEditor::resetParams
            );

            this->_resetButton->setEnabled(false);
        }

        AtomEditor* editor() {
            return this->_editor;
        }
    
    private slots:
        void _handleAlterationRequest(const AlterationPayload &payload) {
            
            auto casted = Payloads::autoCast(payload);

            //if ghost item selection occured
            if(auto mPayload = dynamic_cast<AtomTemplateSelectedPayload*>(casted.data())) {
                
                auto atom_template = mPayload->selectedTemplate();
                AtomsSelectionDescriptor descr;
                
                if(!atom_template.isEmpty()) {

                    descr.representedTypes.insert(
                        atom_template.type()
                    );

                    descr.templateAtom = atom_template;

                }

                this->_handleSubjectChange(descr);
                
            }

            //if selection occured
            else if(auto mPayload = dynamic_cast<SelectedPayload*>(casted.data())) {
                auto selected = mPayload->targetRPZAtomIds();
                auto descr = this->_storage->getAtomSelectionDescriptor(selected);
                this->_handleSubjectChange(descr);
            }

            //if deletion happened
            else if(auto mPayload = dynamic_cast<RemovedPayload*>(casted.data())) {
                
                auto currSelectionDescr = this->_editor->currentSelectionDescriptor();

                auto removed = mPayload->targetRPZAtomIds().toList().toSet();
                auto current = currSelectionDescr.selectedAtomIds.toList().toSet();
                
                //if no previous selection and has template atom, keep current description
                if(!currSelectionDescr.templateAtom.isEmpty() && !current.count()) return;

                auto truncated = current.subtract(removed).toList().toVector();
                auto descr = this->_storage->getAtomSelectionDescriptor(truncated);

                this->_handleSubjectChange(descr);
            }
            
            //on reset
            else if(auto mPayload = dynamic_cast<RemovedPayload*>(casted.data())) {
                AtomsSelectionDescriptor descr;
                this->_handleSubjectChange(descr);
            }

        }

};