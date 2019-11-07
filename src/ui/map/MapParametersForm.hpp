#pragma once

#include <QDialog>
#include <QFormLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QIcon>

#include "src/shared/hints/AtomsStorage.h"

class MapParametersForm : public QDialog {
    
    public:
        MapParametersForm(AtomsStorage* toUpdate, QWidget* parent) : QDialog(parent,  Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint), _toUpdate(toUpdate) { 
            
            this->_saveBtn = new QPushButton(tr("Save"));

            this->_minimumZoomScaleSpin = new QDoubleSpinBox;
            this->_minimumZoomScaleSpin->setSuffix("x");

            this->_maximumZoomScaleSpin = new QDoubleSpinBox;
            this->_maximumZoomScaleSpin->setSuffix("x");

            this->_movementSystemCombo = new QComboBox;
            for(auto i = RPZMapParameters::MSToStr.begin(); i != RPZMapParameters::MSToStr.end(); i++) {
                
                //add item
                this->_movementSystemCombo->addItem(
                    QIcon(RPZMapParameters::MSToIcon.value(i.key())), 
                    i.value(), 
                    (int)i.key()
                );
                
                //tooltip
                this->_movementSystemCombo->setItemData(
                    this->_movementSystemCombo->count() - 1,
                    RPZMapParameters::MSToDescr.value(i.key()),
                    Qt::ToolTipRole
                );

            }

            this->_mapSizeSpin = new QSpinBox;

            this->_gridTileToIngameMetersSpin = new QDoubleSpinBox;
            this->_gridTileToIngameMetersSpin->setSuffix(" m");

            this->_gridTileToScreenCentimetersSpin = new QDoubleSpinBox;
            this->_gridTileToScreenCentimetersSpin->setSuffix(" cm");


            this->setSizeGripEnabled(false);
            this->setWindowTitle(tr("Edit map parameters"));
            
            auto layout = new QFormLayout;
            this->setLayout(layout);

            auto minmaxScaleL = new QHBoxLayout;
            minmaxScaleL->addWidget(this->_minimumZoomScaleSpin);
            minmaxScaleL->addWidget(this->_maximumZoomScaleSpin);
            
            layout->addRow(tr("Movement system"), this->_movementSystemCombo);
            layout->addRow(tr("Map size"), this->_mapSizeSpin);
            layout->addRow(tr("Grid tile to ingame meters"), this->_gridTileToIngameMetersSpin);
            layout->addRow(tr("Grid tile to screen centimeters"), this->_gridTileToScreenCentimetersSpin);
            layout->addRow(tr("Minimum / Maximum camera scaling ratio"), minmaxScaleL);
            layout->addRow(this->_saveBtn);
            
        }
    
    private:
        AtomsStorage* _toUpdate = nullptr;
        QPushButton* _saveBtn = nullptr;

        QDoubleSpinBox* _minimumZoomScaleSpin = nullptr;
        QDoubleSpinBox* _maximumZoomScaleSpin = nullptr;
        QComboBox* _movementSystemCombo = nullptr;
        QSpinBox* _mapSizeSpin = nullptr;
        QDoubleSpinBox* _gridTileToIngameMetersSpin = nullptr;
        QDoubleSpinBox* _gridTileToScreenCentimetersSpin = nullptr;
};