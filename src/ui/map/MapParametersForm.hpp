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

#pragma once

#include <QDialog>
#include <QFormLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QIcon>

#include "src/shared/models/RPZMapParameters.hpp"

class MapParametersForm : public QDialog {
    
    public:
        MapParametersForm(const RPZMapParameters &mapParams, QWidget* parent) : 
            QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint) { 
            
            //init
            this->_init();
            this->_load(mapParams);

            //layout
            this->_saveBtn = new QPushButton(QObject::tr("Save and reload map"));
            this->_saveBtn->setAutoDefault(false);
            this->_saveBtn->setDefault(false);
            QObject::connect(
                this->_saveBtn, &QPushButton::pressed,
                [=]() {
                    this->done(QDialog::Accepted);
                }
            );

            this->setSizeGripEnabled(false);
            this->setWindowTitle(QObject::tr("Edit map parameters"));
            
            auto layout = new QFormLayout;
            this->setLayout(layout);

            auto minmaxScaleL = new QHBoxLayout;
            minmaxScaleL->addWidget(this->_minimumZoomScaleSpin);
            minmaxScaleL->addWidget(this->_maximumZoomScaleSpin);
            
            layout->addRow(QObject::tr("Movement system"), this->_movementSystemCombo);
            layout->addRow(QObject::tr("Map size"), this->_mapSizeSpin);
            layout->addRow(QObject::tr("Grid tile to ingame meters"), this->_gridTileToIngameMetersSpin);
            layout->addRow(QObject::tr("Grid tile to screen centimeters"), this->_gridTileToScreenCentimetersSpin);
            layout->addRow(QObject::tr("Minimum / Maximum camera scaling ratio"), minmaxScaleL);
            layout->addRow(this->_saveBtn);
            
        }

        const RPZMapParameters getParametersFromWidgets() {
            
            RPZMapParameters out;

            out.setParameter(RPZMapParameters::Values::MinimumZoomScale, this->_minimumZoomScaleSpin->value());
            out.setParameter(RPZMapParameters::Values::MaximumZoomScale, this->_maximumZoomScaleSpin->value());
            out.setParameter(RPZMapParameters::Values::MapWidth, this->_mapSizeSpin->value());
            out.setParameter(RPZMapParameters::Values::TileToIngameMeters, this->_gridTileToIngameMetersSpin->value());
            out.setParameter(RPZMapParameters::Values::TileToScreenCentimeters, this->_gridTileToScreenCentimetersSpin->value());
            out.setParameter(RPZMapParameters::Values::MovementSystem, this->_movementSystemCombo->currentData().toInt());

            return out;
        }
    
    private:
        QPushButton* _saveBtn = nullptr;

        QDoubleSpinBox* _minimumZoomScaleSpin = nullptr;
        QDoubleSpinBox* _maximumZoomScaleSpin = nullptr;
        QComboBox* _movementSystemCombo = nullptr;
        QSpinBox* _mapSizeSpin = nullptr;
        QDoubleSpinBox* _gridTileToIngameMetersSpin = nullptr;
        QDoubleSpinBox* _gridTileToScreenCentimetersSpin = nullptr;

        void _init() {

            this->_minimumZoomScaleSpin = new QDoubleSpinBox;
            this->_minimumZoomScaleSpin->setSuffix("x");
            this->_minimumZoomScaleSpin->setMinimum(0.001);
            this->_minimumZoomScaleSpin->setMaximum(1);
            this->_minimumZoomScaleSpin->setSingleStep(0.001);
            this->_minimumZoomScaleSpin->setDecimals(3);
           
            this->_maximumZoomScaleSpin = new QDoubleSpinBox;
            this->_maximumZoomScaleSpin->setSuffix("x");
            this->_maximumZoomScaleSpin->setMinimum(1);
            this->_maximumZoomScaleSpin->setMaximum(100);
            this->_maximumZoomScaleSpin->setSingleStep(0.01);

            this->_mapSizeSpin = new QSpinBox;
            this->_mapSizeSpin->setSuffix("pts");
            this->_mapSizeSpin->setMinimum(1000);
            this->_mapSizeSpin->setMaximum(100000);

            this->_gridTileToIngameMetersSpin = new QDoubleSpinBox;
            this->_gridTileToIngameMetersSpin->setSuffix(" m");
            this->_gridTileToIngameMetersSpin->setMinimum(.5);
            this->_gridTileToIngameMetersSpin->setMaximum(5);
            this->_gridTileToIngameMetersSpin->setSingleStep(0.01);

            this->_gridTileToScreenCentimetersSpin = new QDoubleSpinBox;
            this->_gridTileToScreenCentimetersSpin->setSuffix(" cm");
            this->_gridTileToScreenCentimetersSpin->setMinimum(.5);
            this->_gridTileToScreenCentimetersSpin->setMaximum(3);
            this->_gridTileToScreenCentimetersSpin->setSingleStep(0.01);

            this->_movementSystemCombo = new QComboBox;
            for(auto i = RPZMapParameters::MSToStr.begin(); i != RPZMapParameters::MSToStr.end(); i++) {
                
                auto name = QObject::tr(qUtf8Printable(i.value()));
                auto icon = QIcon(RPZMapParameters::MSToIcon.value(i.key()));
                auto tooltip = QObject::tr(qUtf8Printable(RPZMapParameters::MSToDescr.value(i.key())));

                //add item
                this->_movementSystemCombo->addItem(
                    icon, 
                    name, 
                    (int)i.key()
                );
                
                //tooltip
                this->_movementSystemCombo->setItemData(
                    this->_movementSystemCombo->count() - 1,
                    tooltip,
                    Qt::ToolTipRole
                );

            }

        }

        void _load(const RPZMapParameters &mapParams) {
            
            this->_minimumZoomScaleSpin->setValue(mapParams.minimumZoomScale());
            this->_maximumZoomScaleSpin->setValue(mapParams.maximumZoomScale());
            this->_mapSizeSpin->setValue(mapParams.mapWidthInPoints());
            this->_gridTileToIngameMetersSpin->setValue(mapParams.tileToIngameMeters());
            this->_gridTileToScreenCentimetersSpin->setValue(mapParams.tileToScreenCentimeters());
            
            auto index = this->_movementSystemCombo->findData((int)mapParams.movementSystem());
            this->_movementSystemCombo->setCurrentIndex(index);

        }

};