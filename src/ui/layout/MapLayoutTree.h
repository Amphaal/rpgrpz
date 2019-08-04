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

class MapLayoutTree : public RPZTree {

    public:
        MapLayoutTree(AtomsStorage* mapMaster, QWidget* parent = nullptr);
        TreeMapHint* hints();

    protected:
        void keyPressEvent(QKeyEvent * event) override;
        void contextMenuEvent(QContextMenuEvent *event) override;

    private slots:
        void _insertTreeWidgetItem(QTreeWidgetItem *item, QTreeWidgetItem* parent);

    private:
        TreeMapHint* _hints = nullptr;
        void _handleHintsSignalsAndSlots();

        //id fetching
        snowflake_uid _extractAtomIdFromItem(QTreeWidgetItem* item) const;
        QVector<snowflake_uid> _extractAtomIdFromItems(const QList<QTreeWidgetItem*> &items) const;
};