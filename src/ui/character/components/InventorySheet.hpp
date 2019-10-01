#pragma once

#include <QTableWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QHeaderView>
#include <QMessageBox>
#include <QAction>
#include <QKeyEvent>
#include <QMenu>

#include "src/shared/models/character/RPZCharacter.hpp"

class InventorySheet : public QTableWidget {
    public:
        InventorySheet() : QTableWidget(0, 5) {

            this->_addRowAction = new QAction(QIcon(":/icons/app/other/add.png"), "Ajouter un objet"); 
            this->_removeRowAction = new QAction(QIcon(":/icons/app/tools/bin.png"), "Supprimer les objets selectionnées");
            this->_moveItemsAction = new QAction(QIcon(":/icons/app/other/move.png"), "Déplacer les objects selectionnés");
            this->_splitItemAction = new QAction(QIcon(":/icons/app/other/split.png"), "Diviser l'objet selectionné");
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
                RPZInventorySlot ab;

                //name
                auto nameItem = this->item(row, 0);
                if(nameItem) ab.setName(nameItem->text());

                //category
                auto categoryItem = this->item(row, 1);
                if(categoryItem) ab.setCategory(categoryItem->text());
                
                //how many
                auto hmItem = this->item(row, 2);
                if(hmItem) ab.setHowMany(hmItem->data(Qt::EditRole).toInt());
                
                //weight
                auto weightItem = this->item(row, 3); 
                if(weightItem) ab.setWeight(weightItem->data(Qt::EditRole).toDouble());

                //descr
                auto descrItem = this->item(row, 4);
                if(descrItem) ab.setDescription(descrItem->text());

                out.insert(ab.name(), ab);
            }

            toUpdate.setInventorySlots(out);

        }   

        void loadInventory(const RPZInventory &toLoad) {
                     
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
            
            auto selectedItemCount = this->selectedIndexes().count();
            bool singleSelection = selectedItemCount == 1;
            
            this->_removeRowAction->setEnabled(selectedItemCount);
            this->_moveItemsAction->setEnabled(selectedItemCount);
            this->_splitItemAction->setEnabled(singleSelection);
            
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
        QAction* _addRowAction = nullptr;
        QAction* _removeRowAction = nullptr;
        QAction* _moveItemsAction = nullptr;
        QAction* _splitItemAction = nullptr;

        void _moveInventoryItem() {
            //TODO
        }
        
        void _splitInventoryItem() {
            //TODO
        }

        
        void _removeSelection() {
           
            //check if selection is done
            auto selected = this->selectedIndexes();
            if(!selected.count()) return;

            //ask for validation
            auto result = QMessageBox::warning(
                this, 
                "Suppression de(s) objet(s)", 
                "Voulez-vous vraiment supprimer ce(s) objet(s) ?", 
                QMessageBox::Yes|QMessageBox::No, 
                QMessageBox::No
            );
            if(result != QMessageBox::Yes) return;
            
            //get rows to delete and desc order them
            QVector<int> rowsToDelete;
            for(auto &index : selected) {rowsToDelete += index.row();}
            std::sort(rowsToDelete.begin(), rowsToDelete.end(), std::greater<int>());
            
            //remove rows
            for(auto &row : rowsToDelete) {
                this->removeRow(row);
            }
            
        }

        void _addRow(const RPZInventorySlot &iventoryItem = RPZInventorySlot()) {

            auto row = this->rowCount();
            this->insertRow(row);

            //name
            auto nWidget = new QTableWidgetItem(iventoryItem.name());
            this->setItem(row, 0, nWidget);

            //category
            auto cWidget = new QTableWidgetItem(iventoryItem.category());
            this->setItem(row, 1, cWidget);

            //how many
            auto hmWidget = new QTableWidgetItem;
            hmWidget->setData(Qt::EditRole, iventoryItem.howMany());
            this->setItem(row, 2, hmWidget);
            
            //weight
            auto wWidget = new QTableWidgetItem;
            wWidget->setData(Qt::EditRole, iventoryItem.weight());
            this->setItem(row, 3, wWidget);

            //descr
            auto dWidget = new QTableWidgetItem(iventoryItem.description());
            this->setItem(row, 4, dWidget);

        }

};

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

        void loadInventory(const RPZInventory &toLoad) {
                     
            this->_inventoryNameEdit->setText(toLoad.name());
            this->_inventory->loadInventory(toLoad);

        }
    
    private:
        QLineEdit* _inventoryNameEdit = nullptr;
        InventorySheet* _inventory = nullptr;
};
