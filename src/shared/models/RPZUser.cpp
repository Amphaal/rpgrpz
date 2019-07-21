#include "RPZUser.h"
        
RPZUser::RPZUser() {};
RPZUser::RPZUser(const QVariantHash &hash) : Serializable(hash) {}

RPZUser::RPZUser(JSONSocket* socket) : Serializable(SnowFlake::get()->nextId()) {
    this->_associatedSocket = socket;
    this->_localAddress = socket->socket()->localAddress().toString();
    this->_setColor();
};

RPZUser::RPZUser(snowflake_uid id, const QString name, const Role &role, const QColor &color) : Serializable(id) { 
    this->setName(name);
    this->setRole(role);
    this->_setColor(color);
};

void RPZUser::setName(const QString &name) {
    this->insert("name", name);
};

void RPZUser::setRole(const Role &role) {
    this->insert("role", (int)role);
};

JSONSocket* RPZUser::networkSocket() { return this->_associatedSocket; };

QString RPZUser::name() { 

    auto name = this->value("name").toString();
    if(!name.isEmpty()) return name;

    if(!this->_localAddress.isEmpty()) return this->_localAddress;

    return NULL;
};

RPZUser::Role RPZUser::role() {
    return (Role)this->value("role").toInt(); 
};

QColor RPZUser::color() { 
    return QColor(
        this->value("color", "#0000FF").toString()
    ); 
};

QString RPZUser::toString() {
    if(!this->name().isNull()) {
        return this->name();
    } else if (this->id()) {
        return QString::number(this->id());
    } else {
        return "Moi";
    }
}

void RPZUser::_setColor(const QColor &color) {
    auto colorToUse = color.isValid() ? color.name() : RandomColor::getRandomColor().name();
    this->insert("color", colorToUse);
}