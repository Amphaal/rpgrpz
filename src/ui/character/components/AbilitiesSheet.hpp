#pragma once

#include <QTableWidget>
#include <QHeaderView>

class AbilitiesSheet : public QTableWidget {
    public:
        AbilitiesSheet() : QTableWidget(1, 3) {
            this->setSortingEnabled(true);
            this->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);
            this->horizontalHeader()->setStretchLastSection(false);
            this->verticalHeader()->hide();
            this->setHorizontalHeaderLabels({ "Nom", "Catégorie", "Description" });
            this->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeMode::Interactive);
            this->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeMode::ResizeToContents);
            this->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeMode::Stretch);
        }
};