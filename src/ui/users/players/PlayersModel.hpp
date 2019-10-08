#pragma once

#include "../_base/BaseUsersModel.hpp"

class PlayersModel : public BaseUsersModel {
    
    public:
        PlayersModel() { };

        QVariant data(const QModelIndex &index, int role) const override {
            
            if (!index.isValid() || !this->_users.count()) return QVariant();
            
            //check if targeted user exists
            auto expectedId = this->_users.keys().at(index.row());
            auto user = this->_users.value(expectedId);
            if(user.isEmpty()) return QVariant();

            auto character = user.character();
            
            switch(role) {

                case Qt::ToolTipRole: {
                    return QString("%1 en tant que \"%2\"").arg(user.name()).arg(character.name());
                }

                case Qt::UserRole: {
                    return user;
                }

            }

            return QVariant();

        }

    private:
        bool _isUserInvalidForInsert(const RPZUser &user) override {
            return user.role() != RPZUser::Role::Player;
        }
};