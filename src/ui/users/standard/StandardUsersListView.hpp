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