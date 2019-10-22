#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QComboBox>

#include "src/shared/models/character/RPZCharacter.hpp"

#include "src/ui/sheets/components/InventoryEditor.hpp"
#include "src/ui/sheets/components/InventoryPicker.hpp"

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
                this, &InventoriesTab::_onPickerSelectionChanged
            );
            QObject::connect(
                this->_inventoryPicker, &InventoryPicker::requestSave,
                this, &InventoriesTab::_applyInventoryChanges
            );

        }
    
        void updateCharacter(RPZCharacter &toUpdate) {
            
            this->_applyInventoryChanges(
                this->_inventoryPicker->currentInventory()
            );

            this->_inventoryPicker->updateCharacter(toUpdate);

        }   

        void loadCharacter(const RPZCharacter &toLoad, bool isReadOnly) {
            this->_readOnly = isReadOnly;
            this->_inventoryPicker->loadCharacter(toLoad, isReadOnly);
        }

    private:
        bool _readOnly = false;

        InventoryPicker* _inventoryPicker = nullptr;
        InventoryEditor* _inventoryEditor = nullptr;

        void _applyInventoryChanges(RPZInventory* toSave) {
            if(!toSave) return;
            this->_inventoryEditor->updateInventory(*toSave);
        }

        void _onPickerSelectionChanged(const RPZInventory *selected) {
            
            this->_inventoryEditor->setVisible(selected);
            
            if(selected) {
                this->_inventoryEditor->loadInventory(
                    *selected,
                    this->_inventoryPicker->everyInventoriesExceptArg(selected),
                    this->_readOnly
                );
            }

        }
        
};