#pragma once

#include <QTreeWidgetItem>

#include "src/shared/hints/TreeMapHint.h"

class LayerTreeItem : public QTreeWidgetItem {
    public:
        LayerTreeItem();
        bool operator<(const QTreeWidgetItem &other) const override;
};