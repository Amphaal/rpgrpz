#include "src/shared/models/entities/RPZAtom.hpp"

#include "base/AlterationPayload.hpp"

class ResetPayload : public AlterationPayload {
    public:
        ResetPayload(const AlterationPayload::Source &source, QList<RPZAtom> &atoms) : AlterationPayload(AlterationPayload::Alteration::Reset, source) {
            this->_setAddedAtoms(atoms);
        }
            
    QList<RPZAtom> atoms() {
        QList<RPZAtom> out;
        auto list = this->value("atoms").toList();
        for(auto &e : list) {
            out.append(e.toHash());
        }

        return out;
    }

    private:
        void _setAddedAtoms(QList<RPZAtom> &atoms) {
            QVariantList list;
            for(auto &e : atoms) {
                list.append(e);
            }
            (*this)["atoms"] = list;
        }
};