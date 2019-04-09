#pragma once
 
#include <QUuid>
#include <QString>
#include <QVariantHash>
#include <QHostAddress>

#include "src/network/rpz/_any/JSONSocket.h"

#include "Serializable.hpp"

class RPZUser : public Serializable {

    public:
        enum Role { Host, None };
        static const inline QStringList IconsByRoles = { ":/icons/app/connectivity/crown.png", ":/icons/app/connectivity/user.png" };
        
        RPZUser() {};

        RPZUser(JSONSocket* socket) : Serializable(QUuid::createUuid()) {
            this->_jsonHelper = socket;
            this->_localAddress = socket->socket()->localAddress().toString();
        };

        RPZUser(const QUuid &id, const QString name, const Role &role) : Serializable(id), _name(name), _role(role) {

        };

        static RPZUser fromVariantHash(const QVariantHash &hash) {
            return RPZUser(
                hash["id"].toUuid(), 
                hash["name"].toString(), 
                (Role)hash["role"].toInt()
            );
        }; 

        void setName(const QString &name) {
            this->_name = name;
        };

        void setRole(const Role &role) {
            this->_role = role;
        };

        JSONSocket* jsonHelper() { return this->_jsonHelper; };
        QString name() { return this->_name.isEmpty() ? this->_localAddress : this->_name ; };
        Role role() { return this->_role; };

        QVariantHash toVariantHash() override {
            QVariantHash out;

            out.insert("id", this->id());
            out.insert("name", this->name());
            out.insert("role", (int)this->role());

            return out;
        };

    private:
        JSONSocket* _jsonHelper;
        QString _name;
        QString _localAddress;
        Role _role = Role::None;

};