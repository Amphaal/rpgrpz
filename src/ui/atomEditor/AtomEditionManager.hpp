// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical resources available within the source code may 
// use a different license and copyright : please refer to their metadata
// for further details. Graphical resources without explicit references to a
// different license and copyright still refer to this GNU General Public License.

#pragma once

#include "AtomEditor.h"

#include "src/ui/atomEditor/_manager/LayerSelector.h"
#include "src/ui/atomEditor/_manager/HiddenCheckbox.hpp"

#include "src/shared/hints/HintThread.hpp"

class AtomEditionManager : public QWidget {

    Q_OBJECT

    private:
        AtomEditor* _editor = nullptr;
        QPushButton* _resetButton = nullptr;
        HiddenCheckbox* _defaultHiddenCheckbox = nullptr;
        LayerSelector* _layerSelector = nullptr;

        void _handleSubjectChange(const AtomsSelectionDescriptor &atomsSelectDescriptor) {
            this->_resetButton->setEnabled(false);
            this->_editor->buildEditor(atomsSelectDescriptor);
            this->_resetButton->setEnabled(
                this->_editor->hasVisibleEditors()
            );
        }

    public:
        AtomEditionManager(QWidget *parent = nullptr) : QWidget(parent), 
            _editor(new AtomEditor), 
            _resetButton(new QPushButton) {
            
            auto defaultGrpBox = new QGroupBox(QObject::tr("Default configuration"));
            defaultGrpBox->setAlignment(Qt::AlignHCenter);
            auto defaultLayout = new QHBoxLayout;
            defaultGrpBox->setLayout(defaultLayout);
            defaultLayout->setContentsMargins(10, 0, 0, 0);
            this->_layerSelector = new LayerSelector(this);
            this->_defaultHiddenCheckbox = new HiddenCheckbox(this);
            defaultLayout->addWidget(this->_layerSelector);
            defaultLayout->addStretch(1);
            defaultLayout->addWidget(this->_defaultHiddenCheckbox);

            this->_resetButton->setText(tr("Reset displayed parameters"));

            auto layout = new QVBoxLayout;
            this->setLayout(layout);
            
            layout->addWidget(defaultGrpBox, 0);
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

        LayerSelector* layerSelector(){
            return this->_layerSelector;
        }

        HiddenCheckbox* hiddenCheckbox() {
            return this->_defaultHiddenCheckbox;
        }

    
    private slots:
        void _handleAlterationRequest(const AlterationPayload &payload) {
            
            auto casted = Payloads::autoCast(payload);

            //if ghost item selection occured
            if(auto mPayload = dynamic_cast<AtomTemplateSelectedPayload*>(casted.data())) {
                
                auto atom_template = mPayload->selectedTemplate();
                AtomsSelectionDescriptor descr;
                
                if(!atom_template.isEmpty()) {
                    
                    auto type = atom_template.type();
                    descr.representedTypes.insert(type);

                    descr.templateAtom = atom_template;

                }

                this->_handleSubjectChange(descr);
                
            }

            //if selection occured
            else if(auto mPayload = dynamic_cast<SelectedPayload*>(casted.data())) {
                auto selected = mPayload->targetRPZAtomIds();
                auto descr = HintThread::hint()->getAtomSelectionDescriptor(selected);
                this->_handleSubjectChange(descr);
            }

            //if deletion happened
            else if(auto mPayload = dynamic_cast<RemovedPayload*>(casted.data())) {
                
                auto currSelectionDescr = this->_editor->currentSelectionDescriptor();

                auto selected = mPayload->targetRPZAtomIds();
                
                auto removed = QSet<RPZAtom::Id>(selected.begin(), selected.end());
                auto current = QSet<RPZAtom::Id>(currSelectionDescr.selectedAtomIds.begin(), currSelectionDescr.selectedAtomIds.end());
                
                //if no previous selection and has template atom, keep current description
                if(!currSelectionDescr.templateAtom.isEmpty() && !current.count()) return;

                auto truncated = current.subtract(removed).values();
                auto descr = HintThread::hint()->getAtomSelectionDescriptor(truncated);

                this->_handleSubjectChange(descr);
            }
            
            //on reset
            else if(auto mPayload = dynamic_cast<RemovedPayload*>(casted.data())) {
                AtomsSelectionDescriptor descr;
                this->_handleSubjectChange(descr);
            }

        }

};