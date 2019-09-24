#pragma once

#include "src/ui/atoms/base/AtomEditorLineDescriptor.h"
#include <QVBoxLayout>
#include <QComboBox>

#include "src/ui/map/MapView.h"
#include "src/shared/models/RPZAtom.h"
#include "src/ui/atoms/editors/AtomSliderEditor.h"

class BrushToolEditor : public AtomSubEditor {
    private:    
        static inline QMap<int, QString> _strBT {
            { (int)BrushType::Stamp, "Tampon" },
            { (int)BrushType::Rectangle, "Rectangulaire" },
            { (int)BrushType::Ovale, "Circulaire" },
            { (int)BrushType::RoundBrush, "Brosse ronde" },
            { (int)BrushType::Cutter, "Biseau" },
            { (int)BrushType::Scissors, "Ciseaux" }
        };

        static inline QHash<int, QString> _BTicons {
            { (int)BrushType::Stamp, ":/icons/app/tools/stamp.png" },
            { (int)BrushType::Rectangle, ":/icons/app/tools/rectangle.png" },
            { (int)BrushType::Ovale, ":/icons/app/tools/ovale.png" },
            { (int)BrushType::RoundBrush, ":/icons/app/tools/roundBrush.png" },
            { (int)BrushType::Cutter, ":/icons/app/tools/cutter.png" },
            { (int)BrushType::Scissors, ":/icons/app/tools/scissors.png" }
        };

        QComboBox* _combo = nullptr;

    public:
        BrushToolEditor() : AtomSubEditor(AtomParameter::BrushStyle) { 

            this->setVisible(false);

            this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
            
            this->_combo = new QComboBox;
            for(auto i = _strBT.constBegin(); i != _strBT.constEnd(); i++) {

                auto tool = i.key();

                this->_combo->addItem(
                    QIcon(_BTicons[tool]),
                    i.value(),
                    tool
                );

            }

            QObject::connect(
                this->_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                [&](int currentIndex) {
                    auto out = QVariant(currentIndex);
                    emit valueConfirmedForPayload(this->_param, out);
                }
            );

            this->layout()->addWidget(this->_combo);
        };


        void loadTemplate(const QVariant &defaultValue) override {
            
            AtomSubEditor::loadTemplate(defaultValue);

            QSignalBlocker b(this->_combo);
            auto indexToSelect = this->_combo->findData(defaultValue);
            this->_combo->setCurrentIndex(indexToSelect);

        }

};