#pragma once

#include "src/ui/users/_base/BaseUsersModel.hpp"
#include "PlayerItemDelegate.hpp"

class PlayersModel : public BaseUsersModel {
    
    Q_OBJECT

    public:
        PlayersModel() { };

        QVariant data(const QModelIndex &index, int role) const override {
            
            if (!index.isValid() || !this->_users.count()) return QVariant();
            
            if(role == Qt::SizeHintRole) return PlayerItemDelegate::sizeHint(index);

            //check if targeted user exists
            auto expectedId = this->_users.keys().at(index.row());
            
            auto user = this->_users.value(expectedId);
            if(user.isEmpty()) return QVariant();

            switch(role) {

                case Qt::ToolTipRole: {
                    auto character = user.character();
                    return tr("[%1] as \"%2\"").arg(user.name()).arg(character.name());
                }

                case Qt::UserRole: {
                    return user;
                }

            }

            return QVariant();

        }

    private:
        bool _isUserInvalidForInsert(const RPZUser &user) const override {
            return user.role() != RPZUser::Role::Player;
        }
};