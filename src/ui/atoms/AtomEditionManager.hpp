#pragma once

#include "AtomEditor.h"

class AtomEditionManager : public QWidget {
    public:
        AtomEditionManager() : _editor(new AtomEditor), _resetButton(new QPushButton("Valeurs par défaut")) {
            auto layout = new QVBoxLayout;
            this->setLayout(layout);
            
            layout->addWidget(this->_editor, 1);
            layout->addWidget(this->_resetButton);

            QObject::connect(
                this->_resetButton, &QPushButton::pressed,
                this->_editor, &AtomEditor::resetParams
            );

            this->_resetButton->setEnabled(false);
        }

        AtomEditor* editor() {
            return this->_editor;
        }

        void onSubjectedAtomsChange(QVector<void*> &subjectsChanged) {
            QVector<RPZAtom*> list;
            for(auto atom : subjectsChanged) list.append((RPZAtom*)atom);

            this->_resetButton->setEnabled(list.count());

            this->_editor->buildEditor(list);
        }
    
    private:
        AtomEditor* _editor = nullptr;
        QPushButton* _resetButton = nullptr;
};