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

#include <QWidget>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QColorDialog>
#include <QCheckBox>
#include <QLabel>
#include <QMessageBox>

#include "src/shared/models/character/RPZGauge.hpp"
#include "src/ui/sheets/components/SimpleGaugeEditor.hpp"

class GaugeEditor : public SimpleGaugeEditor {

    Q_OBJECT

    public:
        GaugeEditor(const RPZGauge &gauge) : SimpleGaugeEditor({gauge.gaugeValue(), gauge.minGaugeValue(), gauge.maxGaugeValue()}),
            _barNameEdit(new QLineEdit), 
            _colorPicker(new QPushButton),
            _visibleOnPortraitChk(new QCheckBox) {
                
                //bar name 
                auto gNameDescr = tr(" Gauge name");
                this->_barNameEdit->setPlaceholderText(gNameDescr);
                this->_barNameEdit->setToolTip(gNameDescr);
                this->_barNameEdit->setText(gauge.name());
                this->_barNameEdit->setMinimumWidth(80);

                //visible on portrait
                this->_visibleOnPortraitChk->setText(tr("Fav."));
                this->_visibleOnPortraitChk->setToolTip(tr("Visible under portrait ?"));
                this->_visibleOnPortraitChk->setChecked(gauge.isVisibleUnderPortrait());

                //color picker
                this->_colorPicker->setToolTip(tr("Choose a gauge color"));
                QObject::connect(
                    this->_colorPicker, &QPushButton::pressed,
                    this, &GaugeEditor::_onColorPickerPushed
                );
                this->_applyColor(gauge.color());

                //remove bar button
                auto removeBarBtn = new QPushButton;
                removeBarBtn->setIcon(QIcon(QStringLiteral(u":/icons/app/other/remove.png")));
                removeBarBtn->setToolTip(tr("Delete gauge"));
                QObject::connect(
                    removeBarBtn, &QPushButton::pressed,
                    this, &GaugeEditor::_onDeleteButonPressed
                );

                //editors layout
                this->_layoutToInsertTo->insertWidget(0, removeBarBtn);
                this->_layoutToInsertTo->insertWidget(1, this->_colorPicker);
                this->_layoutToInsertTo->insertWidget(2, this->_barNameEdit);
                this->_layoutToInsertTo->insertWidget(3, this->_visibleOnPortraitChk);
                this->_layoutToInsertTo->insertStretch(4, 1);

        }

        RPZGauge toGauge() {
            
            auto minimalist = this->toMinimalist();
            RPZGauge g;

            g.setMinGaugeValue(minimalist.min);
            g.setMaxGaugeValue(minimalist.max);
            g.setGaugeValue(minimalist.current);
            g.setColor(this->_currentColor);
            g.setName(this->_barNameEdit->text()); 
            g.setVisibleUnderPortrait(this->_visibleOnPortraitChk->isChecked());

            return g;

        }
    
    private:
        QLineEdit* _barNameEdit = nullptr;
        QPushButton* _colorPicker = nullptr;
        QCheckBox* _visibleOnPortraitChk = nullptr;

        QColor _currentColor;

        void _applyColor(const QColor &color) {
            
            //define current color
            this->_currentColor = color;
            auto toApply = this->_currentColor.name();

            //apply to color picker
            this->_colorPicker->setStyleSheet(QStringLiteral(u"QPushButton { background-color: %1; }").arg(toApply));

        }

        void _onColorPickerPushed() {
            
            auto selectedColor = QColorDialog::getColor(
                this->_currentColor,
                this,
                tr("Gauge color")
            );
            if(!selectedColor.isValid()) return;

            this->_applyColor(selectedColor);

        }

        void _onDeleteButonPressed() {

            auto result = QMessageBox::warning(
                this, 
                tr("Gauge deletion"), 
                tr("Do you really want to delete this gauge ?"),
                QMessageBox::Yes|QMessageBox::No, 
                QMessageBox::No
            );
            
            if(result != QMessageBox::Yes) return;
            this->deleteLater();

        }
};