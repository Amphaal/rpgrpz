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