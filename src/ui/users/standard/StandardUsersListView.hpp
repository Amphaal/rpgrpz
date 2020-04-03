// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical resources available within the source code may 
// use a different license and copyright : please refer to their metadata
// for further details. Graphical resources without explicit references to a
// different license and copyright still refer to this GNU General Public License.

#pragma once

#include <QListView>

#include "StandardUsersModel.hpp"

#include <QAbstractItemModel>

class StandardUsersListView : public QListView {
    public:
        StandardUsersListView(QWidget *parent = nullptr) : QListView(parent) {
            
            this->setUniformItemSizes(true);
            this->setViewMode(ViewMode::ListMode);
            this->setWordWrap(true);
            this->setMovement(Movement::Static);
            this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
            this->setLayoutDirection(Qt::LayoutDirection::LeftToRight);
            this->setSelectionMode(QAbstractItemView::NoSelection);

            this->setSizeAdjustPolicy(QAbstractScrollArea::SizeAdjustPolicy::AdjustToContents);
            
            this->setVisible(false);

            this->setModel(new StandardUsersModel);

            QObject::connect(
                this->model(), &QAbstractItemModel::rowsRemoved,
                this, &StandardUsersListView::_onRowRemoved
            );
            QObject::connect(
                this->model(), &QAbstractItemModel::modelReset,
                this, &StandardUsersListView::_onRowRemoved
            );
            QObject::connect(
                this->model(), &QAbstractItemModel::rowsInserted,
                this, &StandardUsersListView::_onRowInserted
            );

        }
    
    private:
        void _onRowRemoved() {
            this->setVisible(
                this->model()->rowCount()
            );
        }

        void _onRowInserted() {
            this->setVisible(true);
        }

};