#pragma once

#include <QTableWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QHeaderView>

class InventorySheet : public QWidget {
    public:
        InventorySheet() : _inventoryNameEdit(new QLineEdit), _inventory(new QTableWidget(1, 5)) {

            this->_inventoryNameEdit->setPlaceholderText(" Description / emplacement de l'inventaire (Equipé, Mon sac, Coffre...) ");
            this->_inventoryNameEdit->setContentsMargins(10, 10, 10, 10);

            this->_inventory->setSortingEnabled(true);
            this->_inventory->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);
            this->_inventory->horizontalHeader()->setStretchLastSection(false);
            this->_inventory->verticalHeader()->hide();
            this->_inventory->setHorizontalHeaderLabels({ "Nom", "Catégorie", "Description",  "Pds", "Nbr" });
            this->_inventory->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
            this->_inventory->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
            this->_inventory->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
            this->_inventory->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
            this->_inventory->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);

            // this->_inventory->setItem()

            auto l = new QVBoxLayout;
            l->setMargin(0);
            this->setLayout(l);

            l->addWidget(this->_inventoryNameEdit);
            l->addWidget(this->_inventory, 1);

        }   
    
    private:
        QLineEdit* _inventoryNameEdit = nullptr;
        QTableWidget * _inventory = nullptr;
};