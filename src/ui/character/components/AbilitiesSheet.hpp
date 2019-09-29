#pragma once

#include <QTableWidget>
#include <QHeaderView>

#include "../delegates/CheckBoxDelegate.hpp"
#include "src/shared/models/character/RPZCharacter.hpp"
#include "src/shared/models/character/RPZAbility.hpp"

class AbilitiesSheet : public QTableWidget {
    public:
        AbilitiesSheet() : QTableWidget(0, 4) {
            this->setSortingEnabled(true);

            this->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
            this->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
            
            this->verticalHeader()->hide();
            
            this->setHorizontalHeaderLabels({ "Nom", "Catégorie", "Favoris", "Description" });

            this->horizontalHeader()->setStretchLastSection(false);
            this->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeMode::Interactive);
            this->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeMode::ResizeToContents);
            this->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeMode::ResizeToContents);
            this->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeMode::Interactive);
            this->setItemDelegateForColumn(2, new CheckBoxDelegate);
        }

        void updateCharacter(RPZCharacter &toUpdate) {
            QVector<RPZAbility> out;
            
            for(auto row = 0; row < this->rowCount(); row++) {
                RPZAbility ab;

                //name, skip if name is empty
                auto nameItem = this->item(row, 0);
                if(!nameItem) continue;
                auto name = nameItem->text();
                if(name.isEmpty()) continue;
                ab.setName(name);

                //category
                auto categoryItem = this->item(row, 1);
                if(categoryItem) ab.setCategory(categoryItem->text());
                
                //fav
                auto favItem = this->item(row, 2);
                if(favItem) ab.setFavorite(favItem->data(Qt::EditRole).toBool());

                //descr
                auto descrItem = this->item(row, 3);
                if(descrItem) ab.setDescription(descrItem->text());

                out += ab;
            }

            toUpdate.setAbilities(out);
        }   

        void loadCharacter(const RPZCharacter &toLoad) {
            
            while(this->rowCount() != 0) {
                this->removeRow(0);
            }
            
            for(auto &ability : toLoad.abilities()) {
                this->_addRow(ability);
            }

            this->_addRow();

        }

        void _addRow(const RPZAbility &ability = RPZAbility()) {

            auto row = this->rowCount();
            this->insertRow(row);

            //name
            auto nWidget = new QTableWidgetItem(ability.name());
            this->setItem(row, 0, nWidget);

            //category
            auto cWidget = new QTableWidgetItem(ability.category());
            this->setItem(row, 1, cWidget);

            //is fav
            auto fWidget = new QTableWidgetItem;
            fWidget->setTextAlignment(Qt::AlignCenter);
            fWidget->setData(Qt::DisplayRole, ability.isFavorite());
            this->setItem(row, 2, fWidget);

            //descr
            auto dWidget = new QTableWidgetItem(ability.description());
            this->setItem(row, 3, dWidget);

        }
};