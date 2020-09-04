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

#pragma once

#include <QString>
#include <QDateTime>

#include "src/shared/models/_base/RPZMap.hpp"
#include "src/shared/models/RPZUser.h"
#include "src/shared/models/_base/Stampable.hpp"
#include "src/shared/commands/MessageInterpreter.h"

class RPZMessage : public Stampable {
 public:
    RPZMessage();
    ~RPZMessage();
    explicit RPZMessage(const QVariantHash &hash);
    explicit RPZMessage(const QString &message, const MessageInterpreter::Command &forceCommand = MessageInterpreter::Command::C_Unknown);

    QString text() const;
    MessageInterpreter::Command commandType() const;
    QString toString() const override;
    QPalette palette() const override;
    void setAsLocal();

    void setAsDiceThrowCommand();
    bool isDiceThrowCommand() const;

 private:
    MessageInterpreter::Command _command = MessageInterpreter::Command::C_Unknown;
    bool _isLocal = false;

    void _setText(const QString &text);
    void _forceCommand(const MessageInterpreter::Command &forced);
    void _interpretTextAsCommand();
};
