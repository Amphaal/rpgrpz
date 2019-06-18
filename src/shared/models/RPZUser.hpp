#pragma once
 
#include <QString>
#include <QColor>
#include <QVariantHash>
#include <QHostAddress>

#include "src/network/rpz/_any/JSONSocket.h"

#include "src/helpers/RandomColor.h"
#include "base/Serializable.hpp"

class RPZUser : public Serializable {

    public:
        enum Role { Unknown, Host };
        static const inline QHash<Role, QString> IconsByRoles = { 
            {Unknown, ":/icons/app/connectivity/user.png"},
            {Host, ":/icons/app/connectivity/crown.png"}
        };
        
        RPZUser() {};
        RPZUser(const QVariantHash &hash) : Serializable(hash) {}

        RPZUser(JSONSocket* socket) : Serializable(SnowFlake::get()->nextId()) {
            this->_jsonHelper = socket;
            this->_localAddress = socket->socket()->localAddress().toString();
            this->_setColor();
        };

        RPZUser(const snowflake_uid &id, const QString name, const Role &role, const QColor &color) : Serializable(id) { 
            this->setName(name);
            this->setRole(role);
            this->_setColor(color);
        };

        void setName(const QString &name) {
            this->insert("name", name);
        };

        void setRole(const Role &role) {
            this->insert("role", (int)role);
        };

        JSONSocket* jsonHelper() { return this->_jsonHelper; };
        
        QString name() { 

            auto name = this->value("name").toString();
            if(!name.isEmpty()) return name;

            if(!this->_localAddress.isEmpty()) return this->_localAddress;

            return NULL;
        };

        Role role() {
            return (Role)this->value("role").toInt(); 
        };

        QColor color() { 
            return QColor(
                this->value("color", "#0000FF").toString()
            ); 
        };

        QString toString() {
            if(!this->name().isNull()) {
                return this->name();
            } else if (this->id()) {
                return QString::number(this->id());
            } else {
                return "Moi";
            }
        }

    private:
        void _setColor(const QColor &color = QColor()) {
            auto colorToUse = color.isValid() ? color.name() : RandomColor::getRandomColor().name();
            this->insert("color", colorToUse);
        }

        JSONSocket* _jsonHelper;
        QString _localAddress;

};