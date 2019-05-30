#pragma once

#include <QTreeWidgetItem>

class LayerTreeItem : public QTreeWidgetItem {
    public:
        LayerTreeItem() : QTreeWidgetItem(1001) {}

        bool operator<(const QTreeWidgetItem &other) const override {

            auto othersData = other.data(0, Qt::UserRole);
            if(othersData.isNull() || !othersData.toUuid().isNull()) return false;
            
            auto layer = this->data(0, Qt::UserRole).toInt();
            return layer < othersData.toInt();
        }
};