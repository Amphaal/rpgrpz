#pragma once

#include <QWidget>
#include <QFormLayout>
#include <QSpinBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>

#include "../components/CustomBarGenerator.hpp"
#include "../components/AbilitiesSheet.hpp"

class StatusTab : public QWidget {
    public:
        StatusTab() {
            
            //state tab
            auto stateTabLayout = new QFormLayout;
            stateTabLayout->setSpacing(20);
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
                barsLayout->setSpacing(10);
                bars->setLayout(barsLayout);
                    
                    //default bar
                    barsLayout->addWidget(new CustomBarGenerator);

                    //add bar button
                    auto addBarBtn = new QPushButton;
                    addBarBtn->setIcon(QIcon(":/icons/app/other/add.png"));
                    addBarBtn->setToolTip("Ajouter une nouvelle jauge");
                    barsLayout->addSpacing(10);
                    barsLayout->addWidget(addBarBtn, 0, Qt::AlignRight);
                
                //abilities
                auto abilitiesGrp = new QGroupBox("Compétences");
                abilitiesGrp->setAlignment(Qt::AlignCenter);
                this->_abilitiesSheet = new AbilitiesSheet;
                abilitiesGrp->setLayout(new QVBoxLayout);
                abilitiesGrp->layout()->addWidget(this->_abilitiesSheet);
                stateTabLayout->addRow(abilitiesGrp);
        }
    
    private:
        QLineEdit* _statusBonusEdit = nullptr;
        QLineEdit* _statusMalusEdit = nullptr;
        QSpinBox* _levelSpin = nullptr;
        AbilitiesSheet* _abilitiesSheet = nullptr;
};