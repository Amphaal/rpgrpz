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

#include "PlayersModel.hpp"
#include "PlayerItemDelegate.hpp"

#include <QAbstractItemModel>

class PlayersListView : public QListView {

    Q_OBJECT

    signals:
        void requestingFocusOnCharacter(const RPZCharacter::Id &characterIdToFocus);

    public:
        PlayersListView(QWidget *parent = nullptr) : QListView(parent) {
            
            this->setUniformItemSizes(false);
            this->setViewMode(ViewMode::ListMode);
            this->setWordWrap(false);
            this->setLayoutMode(LayoutMode::SinglePass);
            this->setMovement(Movement::Static);
            this->setResizeMode(ResizeMode::Fixed);
            this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
            this->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);
            this->setFixedWidth(PlayerItemDelegate::DEFAULT_PORTRAIT_SIZE.width() + 2);
            
            this->setContentsMargins(0, 0, 0, 0);
            this->setSpacing(0);
            this->setVisible(false);

            this->setItemDelegate(new PlayerItemDelegate);  
            this->setModel(new PlayersModel);

            QObject::connect(
                this->model(), &QAbstractItemModel::rowsRemoved,
                this, &PlayersListView::_onRowRemoved
            );
            QObject::connect(
                this->model(), &QAbstractItemModel::modelReset,
                this, &PlayersListView::_onRowRemoved
            );
            QObject::connect(
                this->model(), &QAbstractItemModel::rowsInserted,
                this, &PlayersListView::_onRowInserted
            );

            QObject::connect(
                this, &QAbstractItemView::doubleClicked,
                this, &PlayersListView::_onPlayerDoubleClick
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
        
        void _onPlayerDoubleClick(const QModelIndex &index) {
            RPZUser user(index.data(Qt::UserRole).toHash());
            auto characterId = user.character().id();
            emit requestingFocusOnCharacter(characterId);
        }

};