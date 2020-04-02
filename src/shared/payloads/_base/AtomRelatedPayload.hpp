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

#pragma once

#include "AlterationPayload.hpp"

class AtomRelatedPayload : public AlterationPayload {
    public:
        using RemainingAtomIds = int;

        AtomRelatedPayload() {};
        explicit AtomRelatedPayload(const QVariantHash &hash) : AlterationPayload(hash) {};
        AtomRelatedPayload(const Payload::Alteration &type) : AlterationPayload(type) {};

        virtual AtomRelatedPayload::RemainingAtomIds restrictTargetedAtoms(const QSet<RPZAtom::Id> &idsToRemove) = 0;
};