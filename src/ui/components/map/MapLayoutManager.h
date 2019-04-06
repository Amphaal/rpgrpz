#pragma once

#include <QTreeWidget>
#include <QHash>
#include <QUuid>
#include <QGraphicsItem>
#include <QIcon>

#include "MapView.h"
#include "Asset.hpp"

class MapLayoutManager : public QTreeWidget{

    Q_OBJECT

    public:
        MapLayoutManager(QWidget * parent = nullptr);
        void alterTreeElements(QHash<QUuid, Asset> elements, MapView::MapElementEvtState state);
    
    signals:
        void elementsAlterationAsked(QList<QUuid> elementIds, MapView::MapElementEvtState state);

    protected:
        void keyPressEvent(QKeyEvent * event) override;

    private:
        void _onElementSelectionChanged();
        QHash<QUuid, QTreeWidgetItem*> _treeItemsById;
        QList<QUuid> _extractIdsFromSelection();
        bool _externalInstructionPending = false;

        QTreeWidgetItem* _createTreeItem(QUuid id, Asset asset);
};