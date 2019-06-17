#pragma once

#include "src/ui/atoms/base/AtomEditorLineDescriptor.h"
#include <QVBoxLayout>
#include <QComboBox>

#include "src/ui/map/MapView.h"
#include "src/shared/models/RPZAtom.h"
#include "src/ui/atoms/editors/AtomSliderEditor.h"

class BrushToolEditor : public AtomSubEditor {
    
    Q_OBJECT

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
        AtomSliderEditor* _brushToolWidthEditor = nullptr;

    public:
        BrushToolEditor() : AtomSubEditor(AtomParameter::BrushStyle), _brushToolWidthEditor(new AtomSliderEditor(AtomParameter::BrushPenWidth, 1, 500)) { 
            this->setVisible(false);

            this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
            this->setLayout(new QVBoxLayout);

            auto descr = new AtomEditorLineDescriptor(QString("Outil de terrain"), QString(""), QString(":/icons/app/tabs/config.png"));
            this->layout()->addWidget(descr);
            
            this->_combo = new QComboBox;
            for(auto i = _strBT.constBegin(); i != _strBT.constEnd(); i++) {

                auto tool = i.key();

                this->_combo->addItem(
                    QIcon(_BTicons[tool]),
                    i.value()
                );

            }

            QObject::connect(
                this->_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                [&](int currentIndex) {
                    auto isRoundBrush = (BrushType)currentIndex == BrushType::RoundBrush;
                    this->_brushToolWidthEditor->setVisible(isRoundBrush);
                    emit valueConfirmedForPayload(this->_param, QVariant(currentIndex));
                }
            );

            this->layout()->addWidget(this->_combo);
            this->layout()->addWidget(this->_brushToolWidthEditor);
        };

        void reset() {

            this->_combo->setCurrentIndex(0);
             
            if(this->_brushToolWidthEditor->isVisible()) {
                this->_brushToolWidthEditor->slider()->setValue(1);
            }
        }

        AtomSliderEditor* widthEditor() {
            return this->_brushToolWidthEditor;
        }

};