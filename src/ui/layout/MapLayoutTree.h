#pragma once

#include <QTreeWidget>
#include <QSet>
#include <QHash>
#include <QVector>
#include <QGraphicsItem>
#include <QMenu>
#include <QHeaderView>

#include "src/shared/commands/RPZActions.h"

#include "src/shared/commands/RPZActions.h"

#include "src/shared/models/RPZAtom.h"
#include "src/shared/payloads/Payloads.h"

#include "src/ui/map/base/RPZTree.hpp"

#include "src/ui/others/ClientBindable.h"

#include "base/MapLayoutItemDelegate.h"

#include "src/shared/atoms/TreeMapHint.h"

#include "src\shared\commands\AtomsContextualMenuHandler.h"


class MapLayoutTree : public RPZTree, public AtomsContextualMenuHandler {

    public:
        MapLayoutTree(AtomsStorage* mapMaster, QWidget* parent = nullptr);
        TreeMapHint* hints();

    protected:
        void keyPressEvent(QKeyEvent * event) override;
        void contextMenuEvent(QContextMenuEvent *event) override;

    private slots:
        void _insertTreeWidgetItem(QTreeWidgetItem *item, QTreeWidgetItem* parent);
        void _moveFromLayer(QTreeWidgetItem* oldLayerItem, QTreeWidgetItem* newLayerItem, QTreeWidgetItem *item);
        void _renameItem(QTreeWidgetItem* toRename, const QString &newName);
        void _removeItem(QTreeWidgetItem* toRemove);
        void _selectItem(QTreeWidgetItem* toSelect);
        void _clearSelectedItems();
        void _changeItemData(QTreeWidgetItem* target, int column, const QHash<int, QVariant> &newData);

    private:
        TreeMapHint* _hints = nullptr;
        void _handleHintsSignalsAndSlots();

        //id fetching
        RPZAtomId _extractRPZAtomIdFromItem(QTreeWidgetItem* item) const;
        QVector<RPZAtomId> _extractRPZAtomIdFromItems(const QList<QTreeWidgetItem*> &items) const;

        void _updateLayerState(QTreeWidgetItem* layerItem);
};