#pragma once

#include "src/ui/atoms/base/AtomEditorLineDescriptor.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QComboBox>

#include "src/ui/map/MapView.h"
#include "src/ui/atoms/base/BrushToolWidthEditor.hpp"

class BrushToolEditor : public QWidget {
    
    Q_OBJECT

    signals:
        void brushToolChanged(int selectedBrushTool, int brushWidth);

    private:    
        static inline QMap<MapView::BrushTool, QString> _strBT {
            { MapView::Stamp, "Tampon" },
            { MapView::Rectangle, "Rectangulaire" },
            { MapView::Ovale, "Circulaire" },
            { MapView::RoundBrush, "Brosse ronde" },
            //{ MapView::Cutter, "Biseau" },
            { MapView::Scissors, "Ciseaux" }
        };

        static inline QHash<MapView::BrushTool, QString> _BTicons {
            { MapView::Stamp, ":/icons/app/tools/stamp.png" },
            { MapView::Rectangle, ":/icons/app/tools/rectangle.png" },
            { MapView::Ovale, ":/icons/app/tools/ovale.png" },
            { MapView::RoundBrush, ":/icons/app/tools/roundBrush.png" },
            //{ MapView::Cutter, ":/icons/app/tools/cutter.png" },
            { MapView::Scissors, ":/icons/app/tools/scissors.png" }
        };

        QComboBox* _combo = nullptr;
        BrushToolWidthEditor* _brushToolWidthEditor = nullptr;

        void _onBrushToolChanged() {
            emit brushToolChanged(
                this->_combo->currentIndex(), 
                this->_brushToolWidthEditor->spin()->value()
            );
        }

    public:
        BrushToolEditor() : _brushToolWidthEditor(new BrushToolWidthEditor) { 
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
                this->combo(), QOverload<int>::of(&QComboBox::currentIndexChanged),
                [&](int currentIndex) {
                        auto isFreeDraw = (MapView::BrushTool)currentIndex == MapView::FreeDraw;
                        this->_brushToolWidthEditor->setVisible(isFreeDraw);
                        this->_onBrushToolChanged();
                }
            );
            QObject::connect(
                this->_brushToolWidthEditor->spin(), QOverload<int>::of(&QSpinBox::valueChanged),
                this, &BrushToolEditor::_onBrushToolChanged
            );

            this->layout()->addWidget(this->_combo);
            this->layout()->addWidget(this->_brushToolWidthEditor);
        };

        void reset() {

            this->_combo->blockSignals(true);
                this->_combo->setCurrentIndex(0);
            this->_combo->blockSignals(false);

                
            if(this->_brushToolWidthEditor->isVisible()) {
                this->_brushToolWidthEditor->spin()->blockSignals(true);
                    this->_brushToolWidthEditor->spin()->setValue(1);
                this->_brushToolWidthEditor->spin()->blockSignals(false);
            }

            this->_onBrushToolChanged();

        }

        QComboBox* combo() {
            return this->_combo;
        }

};