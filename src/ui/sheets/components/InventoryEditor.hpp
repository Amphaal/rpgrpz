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

// Any graphical resources available within the source code may 
// use a different license and copyright : please refer to their metadata
// for further details. Graphical resources without explicit references to a
// different license and copyright still refer to this GNU General Public License.

#pragma once

#include "InventorySheet.hpp"

class InventoryEditor : public QWidget {

    Q_OBJECT

    public:
        InventoryEditor() : _inventoryNameEdit(new QLineEdit), _inventory(new InventorySheet) {
            
            this->_inventoryNameEdit->setPlaceholderText(
                tr(" Description / location of the inventory (Equiped, My bag, Chest...) ")
            );
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

        void loadInventory(const RPZInventory &toLoad, const QVector<RPZInventory*> &others, bool isReadOnly) {
                     
            this->_inventoryNameEdit->setText(toLoad.name());
            this->_inventoryNameEdit->setReadOnly(isReadOnly);

            this->_inventory->loadInventory(toLoad, others, isReadOnly);

        }
    
    private:
        QLineEdit* _inventoryNameEdit = nullptr;
        InventorySheet* _inventory = nullptr;
};
