#include "LayerTreeItem.h"

LayerTreeItem::LayerTreeItem() : QTreeWidgetItem(1001) {}

bool LayerTreeItem::operator<(const QTreeWidgetItem &other) const {

    auto othersData = other.data(0, Qt::UserRole);
    if(othersData.isNull()) return false;
    
    auto layer = this->data(0, Qt::UserRole).toInt();
    return layer < othersData.toInt();
}
