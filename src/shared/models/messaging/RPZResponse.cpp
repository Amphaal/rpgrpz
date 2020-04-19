// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical or audio resources available within the source code may
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GPL.

#include "RPZResponse.h"

RPZResponse::RPZResponse() : Stampable() {}
RPZResponse::RPZResponse(Stampable::Id answererTo, const ResponseCode &code, const QVariant &data) : Stampable() {
    this->_setResponseCode(code);
    if(answererTo) this->_setAnswerer(answererTo);
    if(!data.isNull()) this->_setResponseData(data);
}
RPZResponse::RPZResponse(const QVariantHash &hash) : Stampable(hash) {}

RPZResponse::ResponseCode RPZResponse::responseCode() const {
    return (ResponseCode)this->value(QStringLiteral(u"r")).toInt();
}

QVariant RPZResponse::responseData() const {
    return this->value(QStringLiteral(u"rdata"));
}

Stampable::Id RPZResponse::answerer() const {
    return this->value(QStringLiteral(u"aswr")).toULongLong();
}

QString RPZResponse::toString() const{
    
    switch(this->responseCode()) {

        case ResponseCode::UnknownCommand: {
            return QObject::tr("Server has not understood your command. Type \"/h\" for help.");
        }

        case ResponseCode::ErrorRecipients: {

            QList<QString> rcpts;
            for(const auto &e : this->responseData().toList()) rcpts.append(e.toString());

            return QObject::tr("Target users could not be found : ") + rcpts.join(", ");
        }

        case ResponseCode::ConnectedToServer: {
            return QObject::tr("Logged to the server (%1)").arg(this->responseData().toString());
        }

        case ResponseCode::Error: {
            return QObject::tr("Error has occured : ") + this->responseData().toString();
        }

        default: {
            return this->responseData().toString();
        }
    };

};

QPalette RPZResponse::palette() const {
    
    //default palette
    auto palette = Stampable::palette();

    //switch by resp code...
    switch(this->responseCode()) {
        
        case ResponseCode::Error:
        case ResponseCode::ErrorRecipients:
            palette.setColor(QPalette::Window, "#f9dad4");
            palette.setColor(QPalette::WindowText, "#FF0000");
            break;

        case ResponseCode::ConnectedToServer:
        case ResponseCode::Status:
            palette.setColor(QPalette::Window, "#71ed55");
            palette.setColor(QPalette::WindowText, "#0f4706");
            break;

        default:
            break;
    }

    return palette;
}

void RPZResponse::_setAnswerer(Stampable::Id answererStampableId) {
    this->insert(QStringLiteral(u"aswr"), QString::number(answererStampableId));
}

void RPZResponse::_setResponseCode(const ResponseCode &code) {
    this->insert(QStringLiteral(u"r"), (int)code);
}

void RPZResponse::_setResponseData(const QVariant &data) {
    this->insert(QStringLiteral(u"rdata"), data);
}