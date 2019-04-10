#pragma once
 
#include <QUuid>
#include <QString>
#include <QColor>
#include <QVariantHash>
#include <QHostAddress>

#include "src/network/rpz/_any/JSONSocket.h"


#include "RandomColor.hpp"
#include "Serializable.hpp"

class RPZUser : public Serializable {

    public:
        enum Role { Host, None };
        static const inline QStringList IconsByRoles = { ":/icons/app/connectivity/crown.png", ":/icons/app/connectivity/user.png" };
        
        RPZUser() {};

        RPZUser(JSONSocket* socket) : Serializable(QUuid::createUuid()) {
            this->_jsonHelper = socket;
            this->_localAddress = socket->socket()->localAddress().toString();
            this->_color = RandomColor::getRandomColor();
        };

        RPZUser(const QUuid &id, const QString name, const Role &role, const QColor &color) : 
            Serializable(id), 
            _name(name), 
            _role(role), 
            _color(color) { };

        static RPZUser fromVariantHash(const QVariantHash &hash) {
            return RPZUser(
                hash["id"].toUuid(), 
                hash["name"].toString(), 
                (Role)hash["role"].toInt(),
                hash["color"].toString()
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
        QColor color() { return this->_color; };

        QVariantHash toVariantHash() override {
            QVariantHash out;

            out.insert("id", this->id());
            out.insert("name", this->name());
            out.insert("role", (int)this->role());
            out.insert("color", this->color().name());

            return out;
        };

    private:
        JSONSocket* _jsonHelper;
        QString _name;
        QString _localAddress;
        QColor _color;
        Role _role = Role::None;

};