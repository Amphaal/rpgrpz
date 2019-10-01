#pragma once

#include <QWidget>
#include <QComboBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>

#include "src/shared/models/character/RPZInventory.hpp"

class InventoryPicker : public QWidget {

    Q_OBJECT

    signals:
        void selectionChanged(const RPZInventory* selected);
        void requestSave(RPZInventory* toSave);

    public:
        InventoryPicker() : _inventoryListCombo(new QComboBox), _deleteInventoryBtn(new QPushButton) {
            
            //add new inventory
            auto newInventoryBtn = new QPushButton;
            newInventoryBtn->setIcon(QIcon(":/icons/app/other/add.png"));
            newInventoryBtn->setToolTip("Créer un nouvel inventaire");
            newInventoryBtn->setMaximumWidth(25);
            QObject::connect(
                newInventoryBtn, &QPushButton::pressed,
                this, &InventoryPicker::_addButtonPressed
            );

            //delete inventory
            this->_deleteInventoryBtn->setToolTip("Supprimer l'inventaire");
            this->_deleteInventoryBtn->setMaximumWidth(25);
            this->_deleteInventoryBtn->setIcon(QIcon(":/icons/app/other/remove.png"));
            this->_deleteInventoryBtn->setEnabled(false);
            QObject::connect(
                this->_deleteInventoryBtn, &QPushButton::pressed,
                this, &InventoryPicker::_deleteButtonPressed
            );

            //on selected inventory changed
            QObject::connect(
                this->_inventoryListCombo, qOverload<int>(&QComboBox::currentIndexChanged),
                this, &InventoryPicker::_onSelectedIndexChanged
            );
            
            //layout
            auto clLayout = new QHBoxLayout;
            this->setLayout(clLayout);
            this->setContentsMargins(0,0,0,0);
            clLayout->addWidget(this->_inventoryListCombo, 1);
            clLayout->addWidget(this->_deleteInventoryBtn);
            clLayout->addWidget(newInventoryBtn);

        }

        RPZInventory* currentInventory() {
            auto id = this->_inventoryListCombo->currentIndex();
            auto inventoriesCount = this->_inventories.count();
            if(id >= inventoriesCount) return nullptr;
            return &this->_inventories[id];
        }

        void updateCharacter(RPZCharacter &character) {
            character.setInventories(this->_inventories);
            this->updateBufferedItemString();
        }

        void loadCharacter(const RPZCharacter &character) {
            this->_loadInventories(character.inventories());
        }

        void updateBufferedItemString() {
            if(!this->_bufferedSelectedInventory) return;

            this->_inventoryListCombo->setItemText(
                this->_bufferedSelectedIndex,
                this->_bufferedSelectedInventory->toString(this->_bufferedSelectedIndex + 1)
            );
        }
    
    private:
        QComboBox* _inventoryListCombo = nullptr;
        QPushButton* _deleteInventoryBtn = nullptr;
        RPZInventory* _bufferedSelectedInventory = nullptr;
        int _bufferedSelectedIndex = -1;
        QIcon _icon = QIcon(":/icons/app/other/bag.png");

        QVector<RPZInventory> _inventories;

        void _addButtonPressed() {
            this->_autoSave();
            this->_inventories.append(RPZInventory());
            this->_loadInventories(this->_inventories);
        }

        void _deleteButtonPressed() {
            
            auto currentChar = this->currentInventory();
            if(!currentChar) return;

            auto result = QMessageBox::warning(
                this, 
                "Suppression d'inventaire", 
                "Voulez-vous vraiment supprimer cet inventaire ?",
                QMessageBox::Yes|QMessageBox::No, 
                QMessageBox::No
            );
            
            if(result == QMessageBox::Yes) {

                this->_inventories.remove(this->_bufferedSelectedIndex);
                
                this->_bufferedSelectedInventory = nullptr;
                this->_bufferedSelectedIndex = -1;
                this->_loadInventories(this->_inventories);

            }

        }

        bool _autoSave() {
            if(!this->_bufferedSelectedInventory) return false;

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
        
        void _loadInventories(const QVector<RPZInventory> &inventories) {
            
            //clear
            {
                QSignalBlocker b(this->_inventoryListCombo);
                this->_inventoryListCombo->clear();
            }
            
            this->_bufferedSelectedInventory = nullptr;
            this->_bufferedSelectedIndex = -1;

            this->_inventories = inventories;
            
            //if no inventory in DB
            if(this->_inventories.isEmpty()) {
                this->_inventoryListCombo->addItem("Aucun inventaire existant, créez en un !");
                this->_inventoryListCombo->setEnabled(false);
                return;
            }

            //add an item for each
            this->_inventoryListCombo->setEnabled(true);
            {
                QSignalBlocker b(this->_inventoryListCombo);
                auto i = 1;
                for(auto &inventory : this->_inventories) {
                    this->_inventoryListCombo->addItem(this->_icon, inventory.toString(i));
                    i++;
                }
            }

            //make sure to trigger event after refresh
            auto last = this->_inventories.count() - 1;
            auto ci = this->_inventoryListCombo->currentIndex();
            if(last != ci) {
                this->_inventoryListCombo->setCurrentIndex(last);
            } else {
                this->_onSelectedIndexChanged(ci);
            }
            
        }


};