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
        void _insertAtomItem(QTreeWidgetItem *item);
        void _renameAtomItem(QTreeWidgetItem* toRename, const QString &newName);
        void _removeItem(QTreeWidgetItem* toRemove);
        void _selectAtomItem(QTreeWidgetItem* toSelect);
        void _updateAtomItemValues(QTreeWidgetItem* toUpdate, const AtomUpdates &updates);
        void _clearSelectedItems();

    private:
        TreeMapHint* _hints = nullptr;
        void _handleHintsSignalsAndSlots();

        //id fetching
        RPZAtomId _extractRPZAtomIdFromItem(QTreeWidgetItem* item) const;
        QVector<RPZAtomId> _extractRPZAtomIdFromItems(const QList<QTreeWidgetItem*> &items) const;

        void _updateLayersDisplayedCount();
};