#pragma once

#include "AtomEditor.h"

class AtomEditionManager : public QWidget {

    Q_OBJECT

    private:
        AtomEditor* _editor = nullptr;
        QPushButton* _resetButton = nullptr;
        AtomsStorage* _storage = nullptr;

        void _handleSubjectChange(const QVector<const RPZAtom*> &newSubjects) {
            bool hasSubjects = newSubjects.count();
            this->_resetButton->setEnabled(hasSubjects);
            this->_editor->buildEditor(newSubjects);
        }

    public:
        AtomEditionManager(AtomsStorage* storage) : _storage(storage), _editor(new AtomEditor), 
            _resetButton(new QPushButton("Valeurs par défaut")) {
            
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

            if(auto mPayload = dynamic_cast<AtomTemplateSelectedPayload*>(casted.data())) {
                auto ptr = mPayload->selectedTemplate();
                if(ptr) this->_handleSubjectChange({ptr});
                else this->_handleSubjectChange({});
                
            }

            else if(auto mPayload = dynamic_cast<SelectedPayload*>(casted.data())) {
                this->_handleSubjectChange(
                    this->_storage->selectedAtoms()
                );
            }

        }

};