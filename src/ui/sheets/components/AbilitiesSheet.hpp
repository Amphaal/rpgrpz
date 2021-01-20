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

#include <QTableWidget>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMessageBox>
#include <QMenu>

#include <algorithm>
#include <functional>

#include "src/ui/sheets/delegates/CheckBoxDelegate.hpp"
#include "src/shared/models/character/RPZCharacter.hpp"
#include "src/shared/models/character/RPZAbility.hpp"

class AbilitiesSheet : public QTableWidget {
    Q_OBJECT

 public:
    AbilitiesSheet() : QTableWidget(0, 4) {
        this->_addRowAction = new QAction(
            QIcon(QStringLiteral(u":/icons/app/other/add.png")),
            tr("Add a new ability")
        );

        this->_removeRowAction = new QAction(
            QIcon(QStringLiteral(u":/icons/app/tools/bin.png")),
            tr("Remove selected abilities")
        );

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

        this->setHorizontalHeaderLabels({
            tr("Name"),
            tr("Category"),
            tr("Favorite"),
            tr("Description")
        });

        this->horizontalHeader()->setStretchLastSection(false);
        this->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeMode::Interactive);
        this->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeMode::ResizeToContents);
        this->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeMode::ResizeToContents);
        this->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeMode::Interactive);

        this->setItemDelegateForColumn(2, new CheckBoxDelegate);
    }

    void updateCharacter(RPZCharacter &toUpdate) {
        QVector<RPZAbility> out;

        for (auto row = 0; row < this->rowCount(); row++) {
            RPZAbility ab;

            // name
            auto nameItem = this->item(row, 0);
            if (nameItem) ab.setName(nameItem->text());

            // category
            auto categoryItem = this->item(row, 1);
            if (categoryItem) ab.setCategory(categoryItem->text());

            // fav
            auto favItem = this->item(row, 2);
            if (favItem) ab.setFavorite(favItem->data(Qt::EditRole).toBool());

            // descr
            auto descrItem = this->item(row, 3);
            if (descrItem) ab.setDescription(descrItem->text());

            out += ab;
        }

        toUpdate.setAbilities(out);
    }

    void loadCharacter(const RPZCharacter &toLoad, bool isReadOnly) {
        this->_readOnly = isReadOnly;

        // clear
        while (this->rowCount() != 0) {
            this->removeRow(0);
        }

        // fill
        for (const auto &ability : toLoad.abilities()) {
            this->_addRow(ability);
        }

        // prevent edition if readOnly
        this->setEditTriggers(isReadOnly ?
            QAbstractItemView::NoEditTriggers :
            QAbstractItemView::EditTrigger::AllEditTriggers
        );
    }

 protected:
    void keyPressEvent(QKeyEvent *event) override {
        switch (event->key()) {
            case Qt::Key_Delete:
                this->_removeSelection();
            break;

            default:
            break;
        }
    }

    void contextMenuEvent(QContextMenuEvent *event) override {
        this->_removeRowAction->setEnabled(!this->_readOnly && this->selectedIndexes().count());
        this->_addRowAction->setEnabled(!this->_readOnly);

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
    bool _readOnly = false;

    QAction* _addRowAction = nullptr;
    QAction* _removeRowAction = nullptr;

    void _removeSelection() {
        if (this->_readOnly) return;

        // check if selection is done
        auto selected = this->selectedIndexes();
        if (!selected.count()) return;

        // ask for validation
        auto result = QMessageBox::warning(
            this,
            tr("Deletion of abilities"),
            tr("Do you really want to delete those abilities ?"),
            QMessageBox::Yes|QMessageBox::No,
            QMessageBox::No
        );
        if (result != QMessageBox::Yes) return;

        // get rows to delete and desc order them
        QSet<int> rowsToDelete;
        for (const auto &index : selected) {rowsToDelete += index.row();}
        auto rowsToDeleteL = rowsToDelete.values();
        std::sort(rowsToDeleteL.begin(), rowsToDeleteL.end(), std::greater<int>());

        // remove rows
        this->setSortingEnabled(false);
        for (const auto &row : rowsToDeleteL) {
            this->removeRow(row);
        }
        this->setSortingEnabled(true);
    }

    void _addRow(const RPZAbility &ability = RPZAbility()) {
        this->setSortingEnabled(false);

        auto row = this->rowCount();
        this->insertRow(row);

        // name
        auto nWidget = new QTableWidgetItem(ability.name());
        this->setItem(row, 0, nWidget);

        // category
        auto cWidget = new QTableWidgetItem(ability.category());
        this->setItem(row, 1, cWidget);

        // is fav
        auto fWidget = new QTableWidgetItem;
        fWidget->setTextAlignment(Qt::AlignCenter);
        fWidget->setData(Qt::EditRole, ability.isFavorite());
        this->setItem(row, 2, fWidget);

        // descr
        auto descr = ability.description();
        auto dWidget = new QTableWidgetItem(descr);
        dWidget->setToolTip(descr);
        this->setItem(row, 3, dWidget);

        this->setSortingEnabled(true);
    }
};
