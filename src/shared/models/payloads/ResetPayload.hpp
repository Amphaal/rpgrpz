#include "src/shared/models/entities/RPZAtom.hpp"

#include "base/AlterationPayload.hpp"

class ResetPayload : public AlterationPayload {
    public:
        ResetPayload(const AlterationPayload::Source &source, QVector<RPZAtom> &atoms) : AlterationPayload(AlterationPayload::Alteration::Reset, source) {
            this->_setAddedAtoms(atoms);
        }
            
    QVector<RPZAtom> atoms() {
        QVector<RPZAtom> out;
        auto list = this->value("atoms").toList();
        for(auto &e : list) {
            out.append(e.toHash());
        }

        return out;
    }

    QVariantHash alterationByAtomId() override {
        QVariantHash out;

        for(auto &atom : this->atoms()) {
            out.insert(atom.id().toString(), atom);
        }
        
        return out;
    }

    private:
        void _setAddedAtoms(QVector<RPZAtom> &atoms) {
            QVariantList list;
            for(auto &e : atoms) {
                list.append(e);
            }
            (*this)["atoms"] = list;
        }
};