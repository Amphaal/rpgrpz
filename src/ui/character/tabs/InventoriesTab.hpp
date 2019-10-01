#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QComboBox>

#include "src/shared/models/character/RPZCharacter.hpp"

#include "../components/InventorySheet.hpp"
#include "../components/InventoryPicker.hpp"

class InventoriesTab : public QWidget {
    public:
        InventoriesTab() : _inventoryPicker(new InventoryPicker), _inventoryEditor(new InventoryEditor) {
           
            auto inventoryTabLayout = new QVBoxLayout;
            this->setLayout(inventoryTabLayout);

            //inventory
            inventoryTabLayout->addWidget(this->_inventoryPicker, 0, Qt::AlignTop);
            inventoryTabLayout->addWidget(this->_inventoryEditor, 1);

             //picker        
            QObject::connect(
                this->_inventoryPicker, &InventoryPicker::selectionChanged,
                [=](const RPZInventory *selected) {
                    this->_inventoryEditor->setVisible(selected);
                    if(selected) this->_inventoryEditor->loadInventory(*selected);
                }
            );
            QObject::connect(
                this->_inventoryPicker, &InventoryPicker::requestSave,
                this, &InventoriesTab::_applyInventoryChanges
            );

        }
    
        void updateCharacter(RPZCharacter &toUpdate) {
            this->_applyInventoryChanges(this->_inventoryPicker->currentInventory());
            this->_inventoryPicker->updateCharacter(toUpdate);
        }   

        void loadCharacter(const RPZCharacter &toLoad) {
            this->_inventoryPicker->loadCharacter(toLoad);
        }

    private:
        InventoryPicker* _inventoryPicker = nullptr;
        InventoryEditor* _inventoryEditor = nullptr;

        void _applyInventoryChanges(RPZInventory* toSave) {
            if(!toSave) return;
            this->_inventoryEditor->updateInventory(*toSave);
        }
        
};