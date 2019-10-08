#pragma once

#include <QListView>

#include "PlayersModel.hpp"
#include "PlayerItemDelegate.hpp"

#include <QAbstractItemModel>

class PlayersListView : public QListView {
    public:
        PlayersListView(QWidget *parent = nullptr) : QListView(parent) {

            this->setVisible(false);

            this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
            this->setLayoutDirection(Qt::LayoutDirection::LeftToRight);

            this->setItemDelegate(new PlayerItemDelegate);
            
            this->setModel(new PlayersModel);

            QObject::connect(
                this->model(), &QAbstractItemModel::modelReset,
                [=]() {
                    this->setVisible(
                        this->model()->rowCount()
                    );
                }
            );

        }
    

};