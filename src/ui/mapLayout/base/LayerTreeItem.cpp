#include "LayerTreeItem.h"

LayerTreeItem::LayerTreeItem() : QTreeWidgetItem(1001) {}

bool LayerTreeItem::operator<(const QTreeWidgetItem &other) const {

    auto othersData = other.data(0, RPZUserRoles::AtomLayer);
    if(othersData.isNull()) return false;
    
    auto layer = this->data(0, RPZUserRoles::AtomLayer).toInt();
    return layer < othersData.toInt();
}
