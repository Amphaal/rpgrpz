#pragma once

#include "src/ui/atomEditor/_base/AtomEditorLineDescriptor.h"
#include <QVBoxLayout>
#include <QComboBox>

#include "src/shared/models/RPZAtom.h"
#include "src/ui/atomEditor/_base/AtomSubEditor.h"

class NPCAttitudeEditor : public AtomSubEditor {

    Q_OBJECT

    private:    
        static inline QMap<RPZAtom::NPCType, QString> _strAttitude {
            { RPZAtom::NPCType::Unknown, QT_TR_NOOP("Unknown") },
            { RPZAtom::NPCType::Neutral, QT_TR_NOOP("Neutral") },
            { RPZAtom::NPCType::Friendly, QT_TR_NOOP("Friendly") },
            { RPZAtom::NPCType::Hostile, QT_TR_NOOP("Hostile") },
        };

        static inline QHash<RPZAtom::NPCType, QString> _AttitudeIcons {
            { RPZAtom::NPCType::Unknown, QStringLiteral(u":/icons/app/attitude/unknown.png") },
            { RPZAtom::NPCType::Neutral, QStringLiteral(u":/icons/app/attitude/neutral.png") },
            { RPZAtom::NPCType::Friendly, QStringLiteral(u":/icons/app/attitude/friendly.png") },
            { RPZAtom::NPCType::Hostile, QStringLiteral(u":/icons/app/attitude/hostile.png") }
        };

        QComboBox* _combo = nullptr;

    public:
        NPCAttitudeEditor() : AtomSubEditor({RPZAtom::Parameter::NPCAttitude}) { 

            this->setVisible(false);

            this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
            
            this->_combo = new QComboBox;
            for(auto i = _strAttitude.constBegin(); i != _strAttitude.constEnd(); i++) {

                auto tool = i.key();
                
                auto translatedName = tr(qUtf8Printable(i.value()));
                auto associatedIcon = QIcon(_AttitudeIcons.value(tool));

                this->_combo->addItem(associatedIcon, translatedName, (int)tool);

            }

            QObject::connect(
                this->_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                [&](int currentIndex) {
                    auto out = QVariant(currentIndex);
                    emit valueConfirmedForPayload({{this->_params.first(), out}});
                }
            );

            this->layout()->addWidget(this->_combo);
        };

        const AtomSubEditor::FilteredDefaultValues loadTemplate(const RPZAtom::Updates &defaultValues, bool updateMode) override {
            
            auto filtered = AtomSubEditor::loadTemplate(defaultValues, updateMode);

            QSignalBlocker b(this->_combo);
            auto indexToSelect = this->_combo->findData(filtered[this->_params.first()]);
            this->_combo->setCurrentIndex(indexToSelect);

            this->_combo->setEnabled(!updateMode);

            return filtered;
            
        }

};