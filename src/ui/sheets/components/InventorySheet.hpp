#pragma once

#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QHeaderView>
#include <QMessageBox>
#include <QAction>
#include <QKeyEvent>
#include <QMenu>
#include <QDialog>
#include <QComboBox>
#include <QLabel>

#include "src/shared/models/character/RPZCharacter.hpp"
#include "InventoryModal.hpp"

class InventorySheet : public QTableWidget {
    public:
        InventorySheet() : QTableWidget(0, 5) {

            this->_addRowAction = new QAction(QIcon(":/icons/app/other/add.png"), "Ajouter un objet"); 
            this->_removeRowAction = new QAction(QIcon(":/icons/app/tools/bin.png"), "Supprimer les objets selectionnées");
            this->_moveItemsAction = new QAction(QIcon(":/icons/app/other/move.png"), "Déplacer vers...");
            this->_splitItemAction = new QAction(QIcon(":/icons/app/other/split.png"), "Diviser vers...");
            QObject::connect(
                this->_addRowAction, &QAction::triggered,
                [=]() {this->_addRow();}
            );
            QObject::connect(
                this->_removeRowAction, &QAction::triggered,
                [=]() {this->_removeSelection();}
            );
            QObject::connect(
                this->_moveItemsAction, &QAction::triggered,
                [=]() {this->_moveInventoryItem();}
            );
            QObject::connect(
                this->_splitItemAction, &QAction::triggered,
                [=]() {this->_splitInventoryItem();}
            );

            this->setSortingEnabled(true);

            this->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
            this->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

            this->verticalHeader()->hide();

            this->setHorizontalHeaderLabels({ QString::fromUtf8("Nom*"), "Catégorie", "Nbr", "Pds", "Description",});
            this->horizontalHeaderItem(0)->setToolTip("Requis!");

            this->horizontalHeader()->setStretchLastSection(false);
            this->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeMode::Interactive);
            this->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeMode::ResizeToContents);
            this->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeMode::ResizeToContents);
            this->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeMode::ResizeToContents);
            this->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeMode::Interactive);

        }   

        void updateInventory(RPZInventory &toUpdate) {
            
            QMap<QString, RPZInventorySlot> out;
            
            for(auto row = 0; row < this->rowCount(); row++) {
                auto slot = _fromRow(row);
                out.insert(slot.id(), slot);
            }

            toUpdate.setInventorySlots(out);

        }   

        void loadInventory(const RPZInventory &toLoad, const QVector<RPZInventory*> &others) {
            
            this->_inventoryTargets = others;
            this->_self = &toLoad;

            //clear lines
            while(this->rowCount() != 0) {
                this->removeRow(0);
            }

            //add slots
            for(auto &slot : toLoad.inventorySlots()) {
                this->_addRow(slot);
            }

        }

    protected:
        void keyPressEvent(QKeyEvent *event) override {
            switch(event->key()) {
                case Qt::Key_Delete:
                    this->_removeSelection();
                break;

                default:
                break;
            }
        }

        void contextMenuEvent(QContextMenuEvent *event) override {

            auto selectedItemCount = this->_countSelectedRows();
            bool singleSelection = selectedItemCount == 1;
            bool splitable = singleSelection && this->_selectedSlots().first().second.howMany() > 1;
            bool targetsAvailable = this->_inventoryTargets.count();
            
            this->_removeRowAction->setEnabled(selectedItemCount);
            this->_moveItemsAction->setEnabled(selectedItemCount && targetsAvailable);
            this->_splitItemAction->setEnabled(singleSelection && targetsAvailable && splitable);
            
            QMenu menu(this);
            auto pos = this->viewport()->mapToGlobal(
                event->pos()
            );

            menu.addAction(this->_moveItemsAction);
            menu.addAction(this->_splitItemAction);
            menu.addSeparator();
            menu.addAction(this->_removeRowAction);
            menu.addSeparator();
            menu.addAction(this->_addRowAction);
            
            menu.exec(pos);

        }

    private:
        QVector<RPZInventory*> _inventoryTargets;
        const RPZInventory* _self = nullptr;

        QAction* _addRowAction = nullptr;
        QAction* _removeRowAction = nullptr;
        QAction* _moveItemsAction = nullptr;
        QAction* _splitItemAction = nullptr;

        void _moveInventoryItem() {
            
            //ask via dialog
            InventorySlotMoveModal modal(this->_self, this->_inventoryTargets, this->_selectedSlots());
            auto validated = modal.exec();
            if(validated != QDialog::Accepted) return;

            //delete rows
            this->_removeSelection(false);
            
            //extract...
            auto targetSlots = modal.getSelectedTargetInventory()->inventorySlots();
            auto tba = modal.toBeAltered();
            
            //update target
            for(auto i = tba.begin(); i != tba.end(); i++) {
                
                auto id = i.key();
                auto slot = i.value().second;
                
                //just insert if not in target
                if(!targetSlots.contains(id)) {
                    targetSlots.insert(id, slot);
                } 
                
                //else, update howmany count
                else {
                    auto targetSlot = targetSlots.value(id);
                    targetSlot.setHowMany(
                        targetSlot.howMany() + slot.howMany()
                    );
                    targetSlots.insert(id, targetSlot);
                }

            }

            //replace...
            modal.getSelectedTargetInventory()->setInventorySlots(targetSlots);

        }
        
        void _splitInventoryItem() {

            auto toUpdate = this->_selectedSlots().first();

            //ask via dialog
            InventorySlotSplitModal modal(this->_self, this->_inventoryTargets, toUpdate);
            auto validated = modal.exec();
            if(validated != QDialog::Accepted) return;

            //update row
            this->setSortingEnabled(false);
            this->item(modal.toBeAltered().first, 2)->setData(Qt::EditRole, modal.newHowMany());
            this->setSortingEnabled(true);

            //extract all slots from target...
            auto targetSlots = modal.getSelectedTargetInventory()->inventorySlots();

                //check if recipient exists in target
                auto targetSlot = targetSlots.value(modal.toBeAltered().second.name());
                if(!targetSlot.isEmpty()) {
                    targetSlot.setHowMany(
                        targetSlot.howMany() + modal.movingItemsCount()
                    );
                } 

                //if does not exist
                else {
                    targetSlot = modal.toBeAltered().second;
                    targetSlot.setHowMany(modal.movingItemsCount());
                }

            //update
            targetSlots.insert(targetSlot.id(), targetSlot);
            modal.getSelectedTargetInventory()->setInventorySlots(targetSlots);

        }

        int _countSelectedRows() {
            auto selectedRows = 0;
            for(auto &range : this->selectedRanges()) selectedRows += range.rowCount();
            return selectedRows;
        }

        QSet<int> _selectedRows() {
            QSet<int> out;
            for(auto &index : this->selectedIndexes()) out += index.row();
            return out;
        }

        QVector<QPair<int, RPZInventorySlot>> _selectedSlots() {
            QVector<QPair<int, RPZInventorySlot>> out;
            for(auto row : this->_selectedRows()) {
                out += { row, this->_fromRow(row) };
            }
            return out;
        }
        
        void _removeSelection(bool askUser = true) {
           
            //check if selection is done
            auto selected = this->selectedIndexes();
            if(!selected.count()) return;

            if(askUser) {
                //ask for validation
                auto result = QMessageBox::warning(
                    this, 
                    "Suppression de(s) objet(s)", 
                    "Voulez-vous vraiment supprimer ce(s) objet(s) ?", 
                    QMessageBox::Yes|QMessageBox::No, 
                    QMessageBox::No
                );
                if(result != QMessageBox::Yes) return;
            }
            
            //get rows to delete and desc order them
            QSet<int> rowsToDelete;
            for(auto &index : selected) {rowsToDelete += index.row();}
            auto rowsToDeleteL = rowsToDelete.toList();
            std::sort(rowsToDeleteL.begin(), rowsToDeleteL.end(), std::greater<int>());
            
            //remove rows
            this->setSortingEnabled(false);
            for(auto &row : rowsToDeleteL) {
                this->removeRow(row);
            }
            this->setSortingEnabled(true);
            
        }

        RPZInventorySlot _fromRow(int row) {
            
            RPZInventorySlot out;    
                
                //name
                auto nameItem = this->item(row, 0);
                if(nameItem) out.setName(nameItem->text());

                //category
                auto categoryItem = this->item(row, 1);
                if(categoryItem) out.setCategory(categoryItem->text());
                
                //how many
                auto hmItem = this->item(row, 2);
                if(hmItem) out.setHowMany(hmItem->data(Qt::EditRole).toInt());
                
                //weight
                auto weightItem = this->item(row, 3); 
                if(weightItem) out.setWeight(weightItem->data(Qt::EditRole).toDouble());

                //descr
                auto descrItem = this->item(row, 4);
                if(descrItem) out.setDescription(descrItem->text());
            
            return out;

        }

        void _addRow(const RPZInventorySlot &inventoryItem = RPZInventorySlot()) {
            
            this->setSortingEnabled(false);

            auto row = this->rowCount();
            this->insertRow(row);

            //name
            auto nWidget = new QTableWidgetItem(inventoryItem.name());
            this->setItem(row, 0, nWidget);

            //category
            auto cWidget = new QTableWidgetItem(inventoryItem.category());
            this->setItem(row, 1, cWidget);

            //how many
            auto hmWidget = new QTableWidgetItem;
            hmWidget->setData(Qt::EditRole, inventoryItem.howMany());
            this->setItem(row, 2, hmWidget);
            
            //weight
            auto wWidget = new QTableWidgetItem;
            wWidget->setData(Qt::EditRole, inventoryItem.weight());
            this->setItem(row, 3, wWidget);

            //descr
            auto dWidget = new QTableWidgetItem(inventoryItem.description());
            this->setItem(row, 4, dWidget);

            this->setSortingEnabled(true);

        }

};
