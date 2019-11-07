#pragma once

#include <QListView>

#include "PlayersModel.hpp"
#include "PlayerItemDelegate.hpp"

#include <QAbstractItemModel>

class PlayersListView : public QListView {

    Q_OBJECT

    signals:
        void requestingFocusOnCharacter(const SnowFlake::Id &characterIdToFocus);

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
                this->model(), &QAbstractItemModel::modelReset,
                this, &PlayersListView::_onModelReset
            );

            QObject::connect(
                this, &QAbstractItemView::doubleClicked,
                this, &PlayersListView::_onPlayerDoubleClick
            );

        }

    private:
        void _onModelReset() {
            this->setVisible(
                this->model()->rowCount()
            );
        }
        
        void _onPlayerDoubleClick(const QModelIndex &index) {
            RPZUser user(index.data(Qt::UserRole).toHash());
            auto characterId = user.character().id();
            emit requestingFocusOnCharacter(characterId);
        }

};