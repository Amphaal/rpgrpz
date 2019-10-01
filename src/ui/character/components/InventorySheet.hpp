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

class InventorySlotModal : public QDialog {
    public:

        InventorySlotModal(const RPZInventory* from, const QVector<RPZInventory*> &targets) : 
            _targets(targets), 
            _targetsCombo(new QComboBox), 
            _recipientCombo(new QComboBox) {
            
            for(auto target : targets) {
                this->_addInventoryToCombo(this->_targetsCombo, target);
            }
            
            this->_recipientCombo->setEnabled(false);
            this->_addInventoryToCombo(this->_recipientCombo, from);

            auto lTransfert = new QHBoxLayout;
            lTransfert->addWidget(this->_recipientCombo);
            lTransfert->addWidget(new QLabel(">>>"));
            lTransfert->addWidget(this->_targetsCombo);

            auto layout = new QVBoxLayout;
            this->setLayout(layout);
            layout->addLayout(lTransfert);

        }

        QVector<RPZInventorySlot> execForMove() {

        }

        QVector<RPZInventorySlot> execForSplit() {

        }

        int targetsCount() {
            return this->_targets.count();
        };

    private:
        QVector<RPZInventory*> _targets;
        QComboBox* _recipientCombo = nullptr;
        QComboBox* _targetsCombo = nullptr;

        void _addInventoryToCombo(QComboBox* target, const RPZInventory* toAdd) {
            target->addItem(QIcon(":/icons/app/other/bag.png"), toAdd->toString());
        }
};

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
                out.insert(slot.name(), slot);
            }

            toUpdate.setInventorySlots(out);

        }   

        void loadInventory(const RPZInventory &toLoad, const QVector<RPZInventory*> &others) {
            
            if(this->_modal) this->_modal->deleteLater();
            this->_modal = new InventorySlotModal(&toLoad, others);

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
            bool targetsAvailable = this->_modal->targetsCount();
            
            this->_removeRowAction->setEnabled(selectedItemCount);
            this->_moveItemsAction->setEnabled(selectedItemCount && targetsAvailable);
            this->_splitItemAction->setEnabled(singleSelection && targetsAvailable);
            
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
        InventorySlotModal* _modal = nullptr;

        QAction* _addRowAction = nullptr;
        QAction* _removeRowAction = nullptr;
        QAction* _moveItemsAction = nullptr;
        QAction* _splitItemAction = nullptr;

        void _moveInventoryItem() {
            //TODO
            this->_modal->exec();
        }
        
        void _splitInventoryItem() {
            //TODO
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
            QSet<int> rowsToDelete;
            for(auto &index : selected) {rowsToDelete += index.row();}
            auto rowsToDeleteL = rowsToDelete.toList();
            std::sort(rowsToDeleteL.begin(), rowsToDeleteL.end(), std::greater<int>());
            
            //remove rows
            for(auto &row : rowsToDeleteL) {
                this->removeRow(row);
            }
            
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

        void _addRow(const RPZInventorySlot &iventoryItem = RPZInventorySlot()) {
            
            this->setSortingEnabled(false);

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

            this->setSortingEnabled(true);

        }

};
