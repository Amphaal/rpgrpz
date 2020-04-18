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

// Any graphical or audio resources available within the source code may
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GPL.

#pragma once

#include <QWidget>
#include <QFormLayout>
#include <QSpinBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

#include "src/shared/models/character/RPZCharacter.hpp"

#include "src/ui/sheets/components/GaugeEditor.hpp"
#include "src/ui/sheets/components/GaugeWidget.hpp"
#include "src/ui/sheets/components/AbilitiesSheet.hpp"


class StatusTab : public QWidget {
    
    Q_OBJECT
    
    public:
        StatusTab() {
            
            //state tab
            auto stateTabLayout = new QVBoxLayout;     
            this->setLayout(stateTabLayout);
                
                //level
                this->_levelSpin = new QSpinBox;
                this->_levelSpin->setMinimum(-1);
                this->_levelSpin->setValue(-1);

                    //level row...
                    this->_levelRow = new QWidget;
                    auto levelLayout = new QHBoxLayout;
                    levelLayout->setMargin(0);
                    this->_levelRow->setLayout(levelLayout);
                    stateTabLayout->addWidget(this->_levelRow);
                    
                    levelLayout->addWidget(new QLabel(tr("Level")));
                    levelLayout->addWidget(this->_levelSpin, 1);
                     
                //alterations
                auto alterations = new QGroupBox(tr("Alterations"));
                auto alterationsLayout = new QFormLayout;
                alterations->setLayout(alterationsLayout);
                alterations->setAlignment(Qt::AlignCenter);
                stateTabLayout->addWidget(alterations);

                    //character bonus status
                    this->_statusBonusEdit = new QLineEdit;
                    this->_statusBonusEdit->setPlaceholderText(tr(" Under the influence of a potion, filled with determination ..."));
                    alterationsLayout->addRow(tr("State bonus :"), this->_statusBonusEdit);

                    //character malus status
                    this->_statusMalusEdit = new QLineEdit;
                    this->_statusMalusEdit->setPlaceholderText(tr(" Broken leg, poisoned ..."));
                    alterationsLayout->addRow(tr("State malus :"), this->_statusMalusEdit);

                //bars
                auto bars = new QGroupBox(tr("Gauges"));
                bars->setAlignment(Qt::AlignCenter);
                stateTabLayout->addWidget(bars);
                auto barsLayout = new QVBoxLayout;
                barsLayout->setSpacing(20);
                bars->setLayout(barsLayout);
                    
                    //bars container
                    this->_gaugesLayout = new QVBoxLayout;
                    this->_gaugesLayout->setSpacing(5);
                    barsLayout->addLayout(this->_gaugesLayout);

                    //add bar button
                    this->_addBarBtn = new QPushButton;
                    this->_addBarBtn->setIcon(QIcon(QStringLiteral(u":/icons/app/other/add.png")));
                    this->_addBarBtn->setToolTip(tr("Add a new gauge"));
                    QObject::connect(
                        this->_addBarBtn, &QPushButton::pressed,
                        this, &StatusTab::_createGaugeEditorBtnPressed
                    );

                    //descr
                    this->_insertGaugeInv = new QLabel(tr("No gauge bound to the character"));
                    this->_insertGaugeInv->setEnabled(false);

                    //helper
                    auto helperBarLayout = new QHBoxLayout;
                    helperBarLayout->addWidget(this->_insertGaugeInv, 1);
                    helperBarLayout->addWidget(this->_addBarBtn, 0, Qt::AlignRight);
                    barsLayout->addLayout(helperBarLayout);
                
                //abilities
                auto abilitiesGrp = new QGroupBox(tr("Abilities"));
                abilitiesGrp->setAlignment(Qt::AlignCenter);
                this->_abilitiesSheet = new AbilitiesSheet;
                abilitiesGrp->setLayout(new QVBoxLayout);
                abilitiesGrp->layout()->addWidget(this->_abilitiesSheet);
                abilitiesGrp->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
                stateTabLayout->addWidget(abilitiesGrp, 1);
        }
        ~StatusTab() {
            this->_insertGaugeInv = nullptr;
        }

        
        void updateCharacter(RPZCharacter &toUpdate) {
            toUpdate.setLevel(this->_levelSpin->value());
            toUpdate.setBonus(this->_statusBonusEdit->text());
            toUpdate.setMalus(this->_statusMalusEdit->text());
            toUpdate.setGauges(this->_getGaugeValues());
            this->_abilitiesSheet->updateCharacter(toUpdate);
        }   

        void loadCharacter(const RPZCharacter &toLoad, bool isReadOnly) {
            
            //level
            auto level = toLoad.level();
            this->_levelRow->setVisible(!isReadOnly || level > -1);
            this->_levelSpin->setValue(level);
            this->_levelSpin->setReadOnly(isReadOnly); 
            
            //bonus
            this->_statusBonusEdit->setText(toLoad.bonus());
            this->_statusBonusEdit->setReadOnly(isReadOnly);

            //malus
            this->_statusMalusEdit->setText(toLoad.malus());
            this->_statusMalusEdit->setReadOnly(isReadOnly);

            //gauges
            this->_createGauges(
                toLoad.gauges(), 
                isReadOnly
            );
            this->_addBarBtn->setVisible(!isReadOnly); //insert gauge

            //abilities
            this->_abilitiesSheet->loadCharacter(
                toLoad, 
                isReadOnly
            );
            
        }


    private:
        QLineEdit* _statusBonusEdit = nullptr;
        QLineEdit* _statusMalusEdit = nullptr;
        AbilitiesSheet* _abilitiesSheet = nullptr;
        QVBoxLayout* _gaugesLayout = nullptr;
        QLabel* _insertGaugeInv = nullptr;
        QPushButton* _addBarBtn = nullptr;

        QWidget* _levelRow = nullptr;
        QSpinBox* _levelSpin = nullptr;

        QVector<RPZGauge> _getGaugeValues() {
            
            QVector<RPZGauge> out;
            for(auto i = 0; i < this->_gaugesLayout->count(); i++) {
                
                auto widgetPtr = this->_gaugesLayout->itemAt(i)->widget();
                auto gaugeEditor = dynamic_cast<GaugeEditor*>(widgetPtr);
                if(!gaugeEditor) continue;
                
                out += gaugeEditor->toGauge();
            
            }
            return out;

        }

        void _createGauges(const QVector<RPZGauge> &gauges, bool readyOnly) {
            
            //clear previous
            while(this->_gaugesLayout->count()) {
                delete this->_gaugesLayout->itemAt(0)->widget();
            }

            //add new
            for(const auto &gauge : gauges) {
                if(readyOnly) this->_gaugesLayout->addWidget(new GaugeWidget(gauge));
                else this->_createGaugeEditor(gauge);
            }

            //may update indicator
            this->_mayDisplayGaugeCreationIndicator(false);

        }

        void _createGaugeEditor(const RPZGauge &gauge = RPZGauge()) {
           
           auto newBar = new GaugeEditor(gauge);
            QObject::connect(
                newBar, &QObject::destroyed,
                this, &StatusTab::_mayDisplayGaugeCreationIndicator
            );

            this->_gaugesLayout->addWidget(newBar);

        }

        void _createGaugeEditorBtnPressed() {
            this->_createGaugeEditor();
            this->_mayDisplayGaugeCreationIndicator(false);
        }

        void _mayDisplayGaugeCreationIndicator(bool preLayoutCountUpdate = true) {
            if(!this->_insertGaugeInv) return;
            
            auto layoutItemsCount = this->_gaugesLayout->count();
            if(preLayoutCountUpdate) layoutItemsCount--;
            auto mustShowIndicator = !layoutItemsCount;
            this->_insertGaugeInv->setVisible(mustShowIndicator);
            
        }

};