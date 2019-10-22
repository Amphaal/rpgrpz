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

#include "src/ui/map/_base/RPZTree.hpp"

#include "src/ui/_others/ConnectivityObserver.h"

#include "src/ui/mapLayout/_base/MapLayoutItemDelegate.h"

#include "src/shared/hints/TreeMapHint.h"

#include "src/shared/commands/AtomsContextualMenuHandler.h"
#include "src/shared/async-ui/progress/ProgressTracker.hpp"

class MapLayoutTree : public RPZTree {

    public:
        MapLayoutTree(AtomsStorage* mapMaster, QWidget* parent = nullptr);
        ~MapLayoutTree();

        TreeMapHint* hints() const;

    protected:
        void keyPressEvent(QKeyEvent * event) override;
        void contextMenuEvent(QContextMenuEvent *event) override;

    private slots:
        void _onUIAlterationRequest(const PayloadAlteration &type, const QList<QTreeWidgetItem*> &toAlter);
        void _onUIUpdateRequest(const QHash<QTreeWidgetItem*, AtomUpdates> &toUpdate);
        void _onUIUpdateRequest(const QList<QTreeWidgetItem*> &toUpdate, const AtomUpdates &updates);
        void _onUIMoveRequest(const QHash<int, QList<QTreeWidgetItem*>> &childrenMovedToLayer);

    private:
        void _resizeSections();

        AtomsContextualMenuHandler* _menuHandler = nullptr;
        TreeMapHint* _hints = nullptr;
        void _handleHintsSignalsAndSlots();

        //id fetching
        RPZAtomId _extractRPZAtomIdFromItem(QTreeWidgetItem* item) const;
        QVector<RPZAtomId> _extractRPZAtomIdFromItems(const QList<QTreeWidgetItem*> &items) const;

        void _updateLayersDisplayedCount();

        bool _isAssociatedAtomSelectable(QTreeWidgetItem* item);

        //helpers
        void _insertAtomItem(QTreeWidgetItem *item);
        void _renameAtomItem(QTreeWidgetItem* toRename, const QString &newName);
        void _removeItem(QTreeWidgetItem* toRemove);
        void _selectAtomItem(QTreeWidgetItem* toSelect);
        void _updateAtomItemValues(QTreeWidgetItem* toUpdate, const AtomUpdates &updates);
        void _clearSelectedItems();
};