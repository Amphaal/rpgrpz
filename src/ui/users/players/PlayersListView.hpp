#pragma once

#include <QListView>

#include "PlayersModel.hpp"
#include "PlayerItemDelegate.hpp"

#include <QAbstractItemModel>

class PlayersListView : public QListView {

    Q_OBJECT

    signals:
        void requestingFocusOnCharacter(const snowflake_uid &characterIdToFocus);

    public:
        PlayersListView(QWidget *parent = nullptr) : QListView(parent) {
            
            this->setUniformItemSizes(true);
            this->setViewMode(ViewMode::ListMode);
            this->setWordWrap(false);
            this->setLayoutMode(LayoutMode::SinglePass);
            this->setMovement(Movement::Static);
            this->setResizeMode(ResizeMode::Fixed);
            this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
            this->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);
            
            this->setFixedWidth(PlayerItemDelegate::defaultPortraitSize.width() + 2);
            this->setContentsMargins(0, 0, 0, 0);
            this->setSpacing(0);
            this->setVisible(false);

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

            QObject::connect(
                this, &QAbstractItemView::doubleClicked,
                [=](const QModelIndex &index) {
                    RPZUser user(index.data(Qt::UserRole).toHash());
                    emit requestingFocusOnCharacter(user.id());
                }
            );

        }
    
    private:


};