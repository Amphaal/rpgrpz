#pragma once
#include "AtomsHandler.h"

#include "InteractiveMapHint.h"

#include <QTreeWidget>

class TreeMapHint : public AtomsHandler, public InteractiveMapHint {
    public:
        TreeMapHint(QTreeWidget* boundTree) : AtomsHandler(AlterationPayload::Source::Local_MapLayout), _boundTree(boundTree) {
            
        }
    
    private:
        QTreeWidget* _boundTree = nullptr;
};