// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2021 Guillaume Vara <guillaume.vara@gmail.com>

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

#include <QVariantHash>

#include "src/helpers/_appContext.h"

#include "src/shared/models/character/RPZCharacter.hpp"

class RPZHandshake : public QVariantHash {
 public:
    explicit RPZHandshake(const QVariantHash &hash);
    RPZHandshake(const QString &requestedUsername, const RPZCharacter &toIncarnate);

    const QString clientVersion() const;
    const QString requestedUsername() const;
    const RPZCharacter incarnatingAs() const;
};
