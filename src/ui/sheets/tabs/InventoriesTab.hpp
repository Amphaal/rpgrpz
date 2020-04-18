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