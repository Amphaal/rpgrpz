#pragma once

#include <QTreeWidget>
#include <QSet>
#include <QHash>
#include <QVector>
#include <QGraphicsItem>
#include <QIcon>
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

    Q_OBJECT

    public:
        MapLayoutTree(QWidget* parent = nullptr);
        TreeMapHint* hints();

    protected:
        void keyPressEvent(QKeyEvent * event) override;

        void contextMenuEvent(QContextMenuEvent *event) override;

    private:
        TreeMapHint* _hints;
};