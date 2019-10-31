#pragma once

#include "AlterationPayload.hpp"

class AtomRelatedPayload : public AlterationPayload {
    public:
        typedef int RemainingAtomIds;

        AtomRelatedPayload() {};
        explicit AtomRelatedPayload(const QVariantHash &hash) : AlterationPayload(hash) {};
        AtomRelatedPayload(const Payload::Alteration &type) : AlterationPayload(type) {};

        virtual AtomRelatedPayload::RemainingAtomIds restrictTargetedAtoms(const QSet<RPZAtom::Id> &idsToRemove) = 0;
};