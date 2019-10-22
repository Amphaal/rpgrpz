#include "LayerTreeItem.h"

#include "src/helpers/RPZQVariant.hpp"

LayerTreeItem::LayerTreeItem() : QTreeWidgetItem((int)RPZQVariant::Roles::AtomLayer) {}

bool LayerTreeItem::operator<(const QTreeWidgetItem &other) const {

    auto othersLayer = RPZQVariant::atomLayer(&other);
    auto thisLayer = RPZQVariant::atomLayer(this);
    return thisLayer < othersLayer;
    
}
