#pragma once

#include "src/shared/payloads/_base/AtomsWielderPayload.hpp"

class AddedPayload : public AtomsWielderPayload {
    public:
        explicit AddedPayload(const QVariantHash &hash) : AtomsWielderPayload(hash) {}
        AddedPayload(const RPZAtom &atom) : AtomsWielderPayload(QList<RPZAtom>({atom})) {}  
        AddedPayload(const QList<RPZAtom> &atoms) : AtomsWielderPayload(atoms) {}  
};