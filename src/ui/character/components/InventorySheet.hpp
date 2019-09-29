#pragma once

#include <QTableWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QHeaderView>

class InventorySheet : public QWidget {
    public:
        InventorySheet() : _inventoryNameEdit(new QLineEdit), _inventory(new QTableWidget(0, 5)) {

            this->_inventoryNameEdit->setPlaceholderText(" Description / emplacement de l'inventaire (Equipé, Mon sac, Coffre...) ");
            this->_inventoryNameEdit->setContentsMargins(10, 10, 10, 10);

            this->_inventory->setSortingEnabled(true);

            this->_inventory->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
            this->_inventory->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

            this->_inventory->verticalHeader()->hide();

            this->_inventory->setHorizontalHeaderLabels({ "Nom", "Catégorie", "Nbr", "Pds", "Description",});

            this->_inventory->horizontalHeader()->setStretchLastSection(false);
            this->_inventory->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeMode::Interactive);
            this->_inventory->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeMode::ResizeToContents);
            this->_inventory->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeMode::ResizeToContents);
            this->_inventory->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeMode::ResizeToContents);
            this->_inventory->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeMode::Interactive);

            auto l = new QVBoxLayout;
            l->setMargin(0);
            this->setLayout(l);

            l->addWidget(this->_inventoryNameEdit);
            l->addWidget(this->_inventory, 1);

        }   

        void updateCharacter(RPZCharacter &toUpdate) {
            //TODO
        }   

        void loadCharacter(const RPZCharacter &toLoad) {
            //TODO           
            while(this->_inventory->rowCount() != 0) {
                this->_inventory->removeRow(0);
            }

        }
    
    private:
        QLineEdit* _inventoryNameEdit = nullptr;
        QTableWidget* _inventory = nullptr;
};