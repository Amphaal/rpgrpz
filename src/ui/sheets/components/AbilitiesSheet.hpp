#pragma once

#include <QTableWidget>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMessageBox>
#include <QMenu>

#include <algorithm>

#include "../delegates/CheckBoxDelegate.hpp"
#include "src/shared/models/character/RPZCharacter.hpp"
#include "src/shared/models/character/RPZAbility.hpp"

class AbilitiesSheet : public QTableWidget {
    public:
        AbilitiesSheet() : QTableWidget(0, 4) {
            
            this->_addRowAction = new QAction(QIcon(":/icons/app/other/add.png"), "Ajouter une nouvelle compétence"); 
            this->_removeRowAction = new QAction(QIcon(":/icons/app/tools/bin.png"), "Supprimer les compétences selectionnées");
            QObject::connect(
                this->_addRowAction, &QAction::triggered,
                [=]() {this->_addRow();}
            );
            QObject::connect(
                this->_removeRowAction, &QAction::triggered,
                [=]() {this->_removeSelection();}
            );

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

                //name
                auto nameItem = this->item(row, 0);
                if(nameItem) ab.setName(nameItem->text());

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
            
            this->_removeRowAction->setEnabled(this->selectedIndexes().count());
            
            QMenu menu(this);
            auto pos = this->viewport()->mapToGlobal(
                event->pos()
            );

            menu.addAction(this->_removeRowAction);
            menu.addSeparator();
            menu.addAction(this->_addRowAction);
            
            menu.exec(pos);

        }
    
    private:
        QAction* _addRowAction = nullptr;
        QAction* _removeRowAction = nullptr;

        void _removeSelection() {
           
            //check if selection is done
            auto selected = this->selectedIndexes();
            if(!selected.count()) return;

            //ask for validation
            auto result = QMessageBox::warning(
                this, 
                "Suppression de(s) compétence(s)", 
                "Voulez-vous vraiment supprimer ce(s) compétence(s) ?", 
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
            this->setSortingEnabled(false);
            for(auto &row : rowsToDeleteL) {
                this->removeRow(row);
            }
            this->setSortingEnabled(true);
            
        }

        void _addRow(const RPZAbility &ability = RPZAbility()) {
            
            this->setSortingEnabled(false);

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
            fWidget->setData(Qt::EditRole, ability.isFavorite());
            this->setItem(row, 2, fWidget);

            //descr
            auto dWidget = new QTableWidgetItem(ability.description());
            this->setItem(row, 3, dWidget);

            this->setSortingEnabled(true);

        }
};