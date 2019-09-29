#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QComboBox>

#include "src/shared/models/character/RPZCharacter.hpp"
#include "../components/InventorySheet.hpp"

class InventoriesTab : public QWidget {
    public:
        InventoriesTab() : _inventoryListCombo(new QComboBox) {
           
            auto inventoryTabLayout = new QVBoxLayout;
            this->setLayout(inventoryTabLayout);

                //combo inventories
                this->_inventoryListCombo->addItem(QIcon(":/icons/app/other/bag.png"), "Défaut");
                this->_inventoryListCombo->addItem(QIcon(":/icons/app/other/add.png"), "Créer un nouvel inventaire...");
                inventoryTabLayout->addWidget(this->_inventoryListCombo);

                //inventory
                inventoryTabLayout->addWidget(new InventorySheet, 1);
                
        }
    
        void updateCharacter(RPZCharacter &toUpdate) {
            //TODO
        }   

        void loadCharacter(const RPZCharacter &toLoad) {
            //TODO
        }

    private:
        QComboBox* _inventoryListCombo = nullptr;
};