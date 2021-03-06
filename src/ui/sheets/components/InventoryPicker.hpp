// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2021 Guillaume Vara <guillaume.vara@gmail.com>

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
#include <QComboBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>

#include "src/shared/models/character/RPZCharacter.hpp"
#include "src/shared/models/character/RPZInventory.hpp"

class InventoryPicker : public QWidget {
    Q_OBJECT

 signals:
        void selectionChanged(const RPZInventory* selected);
        void requestSave(RPZInventory* toSave);

 public:
        InventoryPicker() : _inventoryListCombo(new QComboBox), _deleteInventoryBtn(new QPushButton), _newInventoryBtn(new QPushButton) {
            // add new inventory
            this->_newInventoryBtn->setIcon(QIcon(QStringLiteral(u":/icons/app/other/add.png")));
            this->_newInventoryBtn->setToolTip(tr("Create new inventory"));
            this->_newInventoryBtn->setMaximumWidth(25);
            QObject::connect(
                this->_newInventoryBtn, &QPushButton::pressed,
                this, &InventoryPicker::_addButtonPressed
            );

            // delete inventory
            this->_deleteInventoryBtn->setToolTip(tr("Delete inventory"));
            this->_deleteInventoryBtn->setMaximumWidth(25);
            this->_deleteInventoryBtn->setIcon(QIcon(QStringLiteral(u":/icons/app/other/remove.png")));
            this->_deleteInventoryBtn->setEnabled(false);
            QObject::connect(
                this->_deleteInventoryBtn, &QPushButton::pressed,
                this, &InventoryPicker::_deleteButtonPressed
            );

            // on selected inventory changed
            QObject::connect(
                this->_inventoryListCombo, qOverload<int>(&QComboBox::currentIndexChanged),
                this, &InventoryPicker::_onSelectedIndexChanged
            );

            // layout
            auto clLayout = new QHBoxLayout;
            this->setLayout(clLayout);
            this->setContentsMargins(0, 0, 0, 0);
            clLayout->addWidget(this->_inventoryListCombo, 1);
            clLayout->addWidget(this->_deleteInventoryBtn);
            clLayout->addWidget(this->_newInventoryBtn);
        }

        RPZInventory* currentInventory() {
            auto id = this->_inventoryListCombo->currentIndex();
            auto inventoriesCount = this->_inventories.count();
            if (id >= inventoriesCount) return nullptr;
            return &this->_inventories[id];
        }

        QVector<RPZInventory*> everyInventoriesExceptArg(const RPZInventory* toExclude) {
            QVector<RPZInventory*> out;

            for (auto &inventory : this->_inventories) {
                auto ptr = &inventory;
                if (toExclude == ptr) continue;
                out += ptr;
            }

            return out;
        }

        void updateCharacter(RPZCharacter &character) {
            character.setInventories(this->_inventories);
            this->updateBufferedItemString();
        }

        void loadCharacter(const RPZCharacter &character, bool isReadOnly) {
            this->_readOnly = isReadOnly;

            this->_loadInventories(character.inventories());

            this->_newInventoryBtn->setVisible(!isReadOnly);
            this->_deleteInventoryBtn->setVisible(!isReadOnly);
        }

        void updateBufferedItemString() {
            if (!this->_bufferedSelectedInventory) return;

            this->_inventoryListCombo->setItemText(
                this->_bufferedSelectedIndex,
                this->_bufferedSelectedInventory->toString()
            );
        }

 private:
        bool _readOnly = false;
        QIcon _icon = QIcon(QStringLiteral(u":/icons/app/other/bag.png"));

        QComboBox* _inventoryListCombo = nullptr;
        QPushButton* _deleteInventoryBtn = nullptr;
        QPushButton* _newInventoryBtn = nullptr;
        RPZInventory* _bufferedSelectedInventory = nullptr;
        int _bufferedSelectedIndex = -1;
        QVector<RPZInventory> _inventories;

        void _addButtonPressed() {
            this->_autoSave();
            this->_inventories.append(RPZInventory());
            this->_reloadInventories();
        }

        void _deleteButtonPressed() {
            auto currentChar = this->currentInventory();
            if (!currentChar) return;

            auto result = QMessageBox::warning(
                this,
                tr("Inventory deletion"),
                tr("Do you really want to delete this inventory ?"),
                QMessageBox::Yes|QMessageBox::No,
                QMessageBox::No
            );

            if (result == QMessageBox::Yes) {
                this->_inventories.remove(this->_bufferedSelectedIndex);

                this->_bufferedSelectedInventory = nullptr;
                this->_bufferedSelectedIndex = -1;
                this->_reloadInventories();
            }
        }

        bool _autoSave() {
            if (!this->_bufferedSelectedInventory) return false;
            if (this->_readOnly) return false;

            emit requestSave(this->_bufferedSelectedInventory);

            this->updateBufferedItemString();

            return true;
        }

        void _onSelectedIndexChanged(int index) {
            this->_autoSave();

            auto newInventory = this->currentInventory();
            this->_deleteInventoryBtn->setEnabled(newInventory);

            this->_bufferedSelectedInventory = newInventory;
            this->_bufferedSelectedIndex = index;

            emit selectionChanged(newInventory);
        }

        void _reloadInventories() {
            this->_loadInventories(this->_inventories);
        }

        void _loadInventories(const QVector<RPZInventory> &inventories) {
            // clear
            {
                QSignalBlocker b(this->_inventoryListCombo);
                this->_inventoryListCombo->clear();
            }

            this->_bufferedSelectedInventory = nullptr;
            this->_bufferedSelectedIndex = -1;

            this->_inventories = inventories;

            // if no inventory in DB
            if (this->_inventories.isEmpty()) {
                this->_inventoryListCombo->addItem(this->_readOnly ?
                    tr("No existing inventory") :
                    tr("No existing inventory, create some !")
                );
                this->_inventoryListCombo->setEnabled(false);
                return;
            }

            // add an item for each
            this->_inventoryListCombo->setEnabled(true);
            {
                QSignalBlocker b(this->_inventoryListCombo);
                for (const auto &inventory : this->_inventories) {
                    this->_inventoryListCombo->addItem(this->_icon, inventory.toString());
                }
            }

            // make sure to trigger event after refresh
            auto last = this->_inventories.count() - 1;
            auto ci = this->_inventoryListCombo->currentIndex();
            if (last != ci) {
                this->_inventoryListCombo->setCurrentIndex(last);
            } else {
                this->_onSelectedIndexChanged(ci);
            }
        }
};
