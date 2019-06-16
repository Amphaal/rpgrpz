#pragma once

#include "src/ui/atoms/base/AtomEditorLineDescriptor.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QComboBox>

#include "src/ui/map/MapView.h"

class BrushToolEditor : public QWidget {
    
    private:    
        static inline QMap<MapView::BrushTool, QString> _strBT {
            { MapView::Stamp, "Tampon" },
            { MapView::Rectangle, "Rectangulaire" },
            { MapView::Ovale, "Circulaire" },
            { MapView::Cutter, "Biseau" },
            { MapView::Scissors, "Ciseaux" }
        };

        static inline QHash<MapView::BrushTool, QString> _BTicons {
            { MapView::Stamp, ":/icons/app/tools/stamp.png" },
            { MapView::Rectangle, ":/icons/app/tools/rectangle.png" },
            { MapView::Ovale, ":/icons/app/tools/ovale.png" },
            { MapView::Cutter, ":/icons/app/tools/cutter.png" },
            { MapView::Scissors, ":/icons/app/tools/scissors.png" }
        };

        QComboBox* _combo = nullptr;

    public:
        BrushToolEditor() { 
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
            this->layout()->addWidget(this->_combo);
        };

        QComboBox* combo() {
            return this->_combo;
        }

};