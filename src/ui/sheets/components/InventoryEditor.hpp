#pragma once

#include "InventorySheet.hpp"

class InventoryEditor : public QWidget {
    public:
        InventoryEditor() : _inventoryNameEdit(new QLineEdit), _inventory(new InventorySheet) {
            
            this->_inventoryNameEdit->setPlaceholderText(" Description / emplacement de l'inventaire (Equipé, Mon sac, Coffre...) ");
            this->_inventoryNameEdit->setContentsMargins(10, 10, 10, 10);
            
            auto l = new QVBoxLayout;
            l->setMargin(0);
            this->setLayout(l);

            l->addWidget(this->_inventoryNameEdit);
            l->addWidget(this->_inventory, 1);

        }

        void updateInventory(RPZInventory &toUpdate) {
            
            toUpdate.setName(this->_inventoryNameEdit->text());
            this->_inventory->updateInventory(toUpdate);

        }   

        void loadInventory(const RPZInventory &toLoad, const QVector<RPZInventory*> &others) {
                     
            this->_inventoryNameEdit->setText(toLoad.name());
            this->_inventory->loadInventory(toLoad, others);

        }
    
    private:
        QLineEdit* _inventoryNameEdit = nullptr;
        InventorySheet* _inventory = nullptr;
};
