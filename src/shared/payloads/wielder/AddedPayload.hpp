#pragma once

#include "src/shared/payloads/_base/AtomsWielderPayload.hpp"

class AddedPayload : public AtomsWielderPayload {
    public:
        AddedPayload(const QVariantHash &hash) : AtomsWielderPayload(hash) { }
        AddedPayload(const MapDatabase &map) : AtomsWielderPayload(PayloadAlteration::Added, map) { }
        
        static AddedPayload fromAtom(const RPZAtom &atom) {
            MapDatabase fakeDb;
            fakeDb.addAtom(atom);
            return AddedPayload(fakeDb);
        }

        static AddedPayload fromAtoms(const QList<RPZAtom> &atoms) {
            MapDatabase fakeDb;
            fakeDb.addAtoms(atoms);
            return AddedPayload(fakeDb);
        }
    
    private:
        const QSet<RPZAssetHash> assetIdsFromAtoms(const QList<RPZAtom> &atoms) const {
            QSet<RPZAssetHash> out;
            for(auto &atom : atoms) out += atom.assetId();
            return out;
        }
};