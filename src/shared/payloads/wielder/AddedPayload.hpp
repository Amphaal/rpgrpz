#pragma once

#include "src/shared/payloads/_base/AtomsWielderPayload.hpp"

class AddedPayload : public AtomsWielderPayload {
    public:
        AddedPayload(const QVariantHash &hash) : AtomsWielderPayload(hash) { }
        AddedPayload(const RPZMap<RPZAtom> &atoms, const QSet<RPZAssetHash> &includedAssetIds) : AtomsWielderPayload(PayloadAlteration::PA_Added, atoms, includedAssetIds) { }
        
        AddedPayload(const RPZAtom &atom) : AddedPayload(
            RPZMap<RPZAtom>(atom), 
            {atom.assetId()}
        ) { }
        
        AddedPayload(const QList<RPZAtom> &atoms) : AddedPayload(
            RPZMap<RPZAtom>(atoms),
            assetIdsFromAtoms(atoms) 
        ) { }
    
    private:
        const QSet<RPZAssetHash> assetIdsFromAtoms(const QList<RPZAtom> &atoms) const {
            QSet<RPZAssetHash> out;
            for(auto &atom : atoms) out += atom.assetId();
            return out;
        }
};