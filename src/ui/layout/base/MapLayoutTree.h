#pragma once

#include <QTreeWidget>
#include <QSet>
#include <QHash>
#include <QVector>
#include <QGraphicsItem>
#include <QIcon>
#include <QMenu>
#include <QHeaderView>

#include "src/shared/command/RPZActions.hpp"

#include "src/shared/command/RPZActions.hpp"

#include "src/shared/models/entities/RPZAtom.hpp"
#include "src/shared/models/Payloads.h"

#include "src/ui/map/base/RPZTree.hpp"

#include "src/ui/others/ClientBindable.hpp"

#include "MapLayoutItemDelegate.hpp"

#include "src/shared/atoms/TreeMapHint.h"

class MapLayoutTree : public RPZTree {

    Q_OBJECT

    public:
        MapLayoutTree(QWidget* parent = nullptr);
        TreeMapHint* hints();

    protected:
        void keyPressEvent(QKeyEvent * event) override;

    private:
        TreeMapHint* _hints;

        void _renderCustomContextMenu(const QPoint &pos);
        void _generateMenu(QList<QTreeWidgetItem*> &itemsToProcess, const QPoint &whereToDisplay);
            QList<QAction*> _genLayerActions(QList<QTreeWidgetItem*> &selectedItems);
            QList<QAction*> _genVisibilityActions(QList<QTreeWidgetItem*> &selectedItems);
            QList<QAction*> _genAvailabilityActions(QList<QTreeWidgetItem*> &selectedItems);

};