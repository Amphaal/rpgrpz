#include "RPZUser.h"
        
RPZUser::RPZUser() {};
RPZUser::RPZUser(const QVariantHash &hash) : Serializable(hash) {}

RPZUser::RPZUser(RPZUserId id, const QString &name, const Role &role, const QColor &color) : Serializable(id) { 
    this->setName(name);
    this->setRole(role);
    this->_setColor(color);
};

void RPZUser::setName(const QString &name) {
    
    //default name
    this->insert(QStringLiteral(u"name"), name);
    
    //whisp name
    auto adapted = MessageInterpreter::usernameToCommandCompatible(name);
    adapted = adapted + this->color().name();
    this->insert(QStringLiteral(u"wname"), adapted);
        
};

void RPZUser::setRole(const Role &role) {
    this->insert(QStringLiteral(u"role"), (int)role);
};


QString RPZUser::name() const { 

    auto name = this->value(QStringLiteral(u"name")).toString();
    if(!name.isEmpty()) return name;

    return NULL;
    
};

RPZUser::Role RPZUser::role() const {
    return (Role)this->value(QStringLiteral(u"role")).toInt(); 
};

QColor RPZUser::color() const { 
    auto colorAsStr = this->value(QStringLiteral(u"color")).toString();
    return colorAsStr.isEmpty() ? QColor() : QColor(colorAsStr); 
};

QString RPZUser::toString() const {
    
    auto name = this->name();
    if(!name.isNull()) {
        return this->name();
    } 
    
    if (auto id = this->id()) {
        return QString::number(id);
    } 
    
    return "Moi";

}

QString RPZUser::whisperTargetName() const {
    return this->value(QStringLiteral(u"wname")).toString();
}

void RPZUser::randomiseColor() {
    this->_setColor();
}

void RPZUser::_setColor(const QColor &color) {
    auto colorToUse = color.isValid() ? color.name() : RandomColor::getRandomColor().name();
    this->insert(QStringLiteral(u"color"), colorToUse);
}

void RPZUser::setCharacter(const RPZCharacter &character) {
    this->insert(QStringLiteral(u"char"), character);
}

const RPZCharacter RPZUser::character() const {
    return RPZCharacter(this->value(QStringLiteral(u"char")).toHash());
}