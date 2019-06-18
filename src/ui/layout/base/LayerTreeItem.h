#pragma once

#include <QTreeWidgetItem>

class LayerTreeItem : public QTreeWidgetItem {
    public:
        LayerTreeItem();
        bool operator<(const QTreeWidgetItem &other) const override;
};