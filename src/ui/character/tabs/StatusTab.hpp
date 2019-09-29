#pragma once

#include <QWidget>
#include <QFormLayout>
#include <QSpinBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>

#include "src/shared/models/character/RPZCharacter.hpp"

#include "../components/GaugeEditor.hpp"
#include "../components/AbilitiesSheet.hpp"

class StatusTab : public QWidget {
    public:
        StatusTab() {
            
            //state tab
            auto stateTabLayout = new QFormLayout;
            stateTabLayout->setSpacing(30);
            this->setLayout(stateTabLayout);
                
                //level
                this->_levelSpin = new QSpinBox;
                this->_levelSpin->setMinimum(-1);
                this->_levelSpin->setValue(-1);
                stateTabLayout->addRow("Niveau", this->_levelSpin);

                //alterations
                auto alterations = new QGroupBox("Altérations");
                auto alterationsLayout = new QFormLayout;
                alterations->setLayout(alterationsLayout);
                alterations->setAlignment(Qt::AlignCenter);
                stateTabLayout->addRow(alterations);

                    //character bonus status
                    this->_statusBonusEdit = new QLineEdit;
                    this->_statusBonusEdit->setPlaceholderText(" Sous l'effet d'une potion, Rempli de détermination ...");
                    alterationsLayout->addRow("Bonus d'état :", this->_statusBonusEdit);

                    //character malus status
                    this->_statusMalusEdit = new QLineEdit;
                    this->_statusMalusEdit->setPlaceholderText(" Jambe cassée, empoisonné...");
                    alterationsLayout->addRow("Malus d'état :", this->_statusMalusEdit);

                //bars
                auto bars = new QGroupBox("Jauges");
                bars->setAlignment(Qt::AlignCenter);
                stateTabLayout->addRow(bars);
                auto barsLayout = new QVBoxLayout;
                barsLayout->setSpacing(20);
                bars->setLayout(barsLayout);
                    
                    //bars container
                    this->_gaugesLayout = new QVBoxLayout;
                    this->_gaugesLayout->setSpacing(5);
                    barsLayout->addLayout(this->_gaugesLayout);

                    //add bar button
                    auto addBarBtn = new QPushButton;
                    addBarBtn->setIcon(QIcon(":/icons/app/other/add.png"));
                    addBarBtn->setToolTip("Ajouter une nouvelle jauge");
                    QObject::connect(
                        addBarBtn, &QPushButton::pressed,
                        this, &StatusTab::_createGaugeBtnPressed
                    );

                    //descr
                    this->_insertGaugeInv = new QLabel("Aucune jauge liée au personnage");
                    this->_insertGaugeInv->setEnabled(false);

                    //helper
                    auto helperBarLayout = new QHBoxLayout;
                    helperBarLayout->addWidget(this->_insertGaugeInv, 1);
                    helperBarLayout->addWidget(addBarBtn, 0, Qt::AlignRight);
                    barsLayout->addLayout(helperBarLayout);
                
                //abilities
                auto abilitiesGrp = new QGroupBox("Compétences");
                abilitiesGrp->setAlignment(Qt::AlignCenter);
                this->_abilitiesSheet = new AbilitiesSheet;
                abilitiesGrp->setLayout(new QVBoxLayout);
                abilitiesGrp->layout()->addWidget(this->_abilitiesSheet);
                stateTabLayout->addRow(abilitiesGrp);
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

        void loadCharacter(const RPZCharacter &toLoad) {
            this->_levelSpin->setValue(toLoad.level());
            this->_statusBonusEdit->setText(toLoad.bonus());
            this->_statusMalusEdit->setText(toLoad.malus());
            this->_createGauges(toLoad.gauges());
            this->_abilitiesSheet->loadCharacter(toLoad);
        }


    private:
        QLineEdit* _statusBonusEdit = nullptr;
        QLineEdit* _statusMalusEdit = nullptr;
        QSpinBox* _levelSpin = nullptr;
        AbilitiesSheet* _abilitiesSheet = nullptr;
        QVBoxLayout* _gaugesLayout = nullptr;
        QLabel* _insertGaugeInv = nullptr;

        QVector<RPZGauge> _getGaugeValues() {
            QVector<RPZGauge> out;
            for(auto i = 0; i < this->_gaugesLayout->count(); i++) {
                auto gaugeEditor = (GaugeEditor*)this->_gaugesLayout->itemAt(i)->widget();
                out += gaugeEditor->toGauge();
            }
            return out;
        }

        void _createGauges(const QVector<RPZGauge> &gauges) {
            
            //clear previous
            for(auto i = 0; i < this->_gaugesLayout->count(); i++) {
                this->_gaugesLayout->itemAt(i)->widget()->deleteLater();
            }

            //add new
            for(auto &gauge : gauges) {
                this->_createGauge(gauge);
            }

            //may update indicator
            this->_mayDisplayGaugeCreationIndicator(false);

        }

        void _createGauge(const RPZGauge &gauge = RPZGauge()) {
           
           auto newBar = new GaugeEditor(gauge);
            QObject::connect(
                newBar, &QObject::destroyed,
                this, &StatusTab::_mayDisplayGaugeCreationIndicator
            );

            this->_gaugesLayout->addWidget(newBar);

        }

        void _createGaugeBtnPressed() {
            this->_createGauge();
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