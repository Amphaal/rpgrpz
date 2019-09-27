#pragma once 

#include <QWidget>
#include <QFormLayout>
#include <QLabel>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QGroupBox>
#include <QScrollArea>
#include <QRegularExpressionValidator>
#include <QRegularExpression>
#include <QTabWidget>
#include <QProgressBar>

#include "components/InventorySheet.hpp"
#include "components/CustomBarGenerator.hpp"

class CharacterSheet : public QWidget {
    public:
        CharacterSheet() {
            
            //define default portrait
            if(!_defaultPortrait) _defaultPortrait = new QPixmap(":/asset/default.jpg");

            //layout
            auto l = new QVBoxLayout;
            this->setLayout(l);

            //list of characters
            this->_characterListCombo = new QComboBox;
            auto newCharacterBtn = new QPushButton;
            newCharacterBtn->setIcon(QIcon(":/icons/app/other/add.png"));
            newCharacterBtn->setToolTip("Créer une nouvelle fiche");
            newCharacterBtn->setMaximumWidth(25);
            auto clLayout = new QHBoxLayout;
            clLayout->addWidget(this->_characterListCombo, 1);
            clLayout->addWidget(newCharacterBtn);
            l->addLayout(clLayout);
            
            //character tab
            auto characterTab = new QWidget;
            auto characterTabLayout = new QFormLayout;
            characterTab->setLayout(characterTabLayout);
            
                //portrait
                auto pLayout = new QVBoxLayout;
                pLayout->setContentsMargins(0, 0, 0, 5);
                characterTabLayout->addRow(pLayout); 

                    //portrait placeholder
                    this->_imgLbl = new QLabel;
                    this->_imgLbl->setPixmap(*_defaultPortrait);
                    this->_imgLbl->setMaximumSize(QSize(240, 320));
                    pLayout->addWidget(this->_imgLbl, 0, Qt::AlignCenter);

                    //portrait change button
                    auto changePortraitBtn = new QPushButton("Changer le portrait");
                    pLayout->addWidget(changePortraitBtn, 0, Qt::AlignCenter);

                //character name
                this->_sheetNameEdit = new QLineEdit;
                characterTabLayout->addRow("Nom de :", this->_sheetNameEdit);
                this->_sheetNameEdit->setPlaceholderText(" Nom usuel du personnage [Requis!]");
                this->_sheetNameEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("\\w")));

                //archtype
                this->_archtypeEdit = new QLineEdit;
                this->_archtypeEdit->setPlaceholderText(" Courte et grossière caractérisation du personnage (Paladin Loyal Bon, Chasseur de Prime...)");
                characterTabLayout->addRow("Archétype :", this->_archtypeEdit);

                //character description
                this->_descriptionEdit = new QTextEdit;
                this->_descriptionEdit->setPlaceholderText("Description physique, psychologique, contextuelle...");
                characterTabLayout->addRow("Description :", this->_descriptionEdit);

                //character story
                this->_storyEdit = new QTextEdit;
                this->_storyEdit->setPlaceholderText("Evolution du personnage au cours de ses aventures...");
                characterTabLayout->addRow("Histoire :", this->_storyEdit);

            //state tab
            auto stateTab = new QWidget;
            auto stateTabLayout = new QFormLayout;
            stateTab->setLayout(stateTabLayout);
                
                //character status
                this->_statusEdit = new QLineEdit;
                this->_statusEdit->setPlaceholderText(" Jambe cassée, empoisonné, sous l'effet d'une potion...");
                stateTabLayout->addRow("Description des altérations :", this->_statusEdit);

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

            //inventory tab
            auto inventoryTab = new QWidget;
            auto invScrollArea = new QScrollArea;
            invScrollArea->setWidget(inventoryTab);
            invScrollArea->setWidgetResizable(true);
            
            auto inventoryTabLayout = new QVBoxLayout;
            inventoryTab->setLayout(inventoryTabLayout);

                //combo inventories
                this->_inventoryListCombo = new QComboBox;
                this->_inventoryListCombo->addItem(QIcon(":/icons/app/other/bag.png"), "Défaut");
                this->_inventoryListCombo->addItem(QIcon(":/icons/app/other/add.png"), "Créer un nouvel inventaire...");
                inventoryTabLayout->addWidget(this->_inventoryListCombo);

                //inventory
                inventoryTabLayout->addWidget(new InventorySheet, 1);

            //tabs
            auto tabs = new QTabWidget;
            tabs->addTab(characterTab, QIcon(":/icons/app/tabs/feather.png"), "Lore");
            tabs->addTab(stateTab, QIcon(":/icons/app/tabs/status.png"), "Statut / Compétences");
            tabs->addTab(invScrollArea, QIcon(":/icons/app/tabs/chest.png"), "Inventaires");
            l->addWidget(tabs);

        };

    private:
        static inline QPixmap* _defaultPortrait = nullptr;

        QLabel* _imgLbl = nullptr;
        QLineEdit* _sheetNameEdit = nullptr;
        QTextEdit* _descriptionEdit = nullptr;
        QTextEdit* _storyEdit = nullptr;
        QLineEdit* _statusEdit = nullptr;
        QComboBox* _characterListCombo = nullptr;
        QComboBox* _inventoryListCombo = nullptr;
        QLineEdit* _archtypeEdit = nullptr;
};
