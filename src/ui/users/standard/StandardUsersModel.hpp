#pragma once

#include "src/ui/users/_base/BaseUsersModel.hpp"
 
class StandardUsersModel : public BaseUsersModel {
    
    public:
        StandardUsersModel() { };

        QVariant data(const QModelIndex &index, int role) const override {
            
            if (!index.isValid() || !this->_users.count()) return QVariant();
            
            //check if targeted user exists
            auto expectedId = this->_users.keys().at(index.row());
            auto user = this->_users.value(expectedId);
            if(user.isEmpty()) return QVariant();
            
            switch(role) {
                
                case Qt::DisplayRole: {
                    return user.name();
                }

                case Qt::DecorationRole: {
                    return QIcon(RPZUser::IconsByRoles.value(user.role()));
                }

            }

            return QVariant();

        }

    private:
        bool _isUserInvalidForInsert(const RPZUser &user) override {
            return user.role() == RPZUser::Role::Player;
        }

};