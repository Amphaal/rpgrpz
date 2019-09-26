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

#include "components/InventorySheet.hpp"

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
                    this->_imgLbl->setAlignment(Qt::AlignCenter);
                    this->_imgLbl->setPixmap(*_defaultPortrait);
                    pLayout->addWidget(this->_imgLbl);

                    //portrait change button
                    auto changePortraitBtn = new QPushButton("Changer le portrait");
                    pLayout->addWidget(changePortraitBtn, 0, Qt::AlignCenter);

                //character name
                this->_sheetNameEdit = new QLineEdit;
                characterTabLayout->addRow("Nom de la fiche :", this->_sheetNameEdit);
                this->_sheetNameEdit->setPlaceholderText("Nom usuel du personnage (Requis)");
                this->_sheetNameEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("\\w")));

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
                this->_statusEdit->setPlaceholderText("Jambe cassée, empoisonné...");
                stateTabLayout->addRow("Etat :", this->_statusEdit);

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
            tabs->addTab(stateTab, QIcon(":/icons/app/tabs/status.png"), "Statut");
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
};
