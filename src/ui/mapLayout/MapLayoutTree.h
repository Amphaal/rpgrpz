#pragma once

#include <QTreeWidget>
#include <QSet>
#include <QHash>
#include <QVector>
#include <QGraphicsItem>
#include <QMenu>
#include <QHeaderView>
#include <QKeyEvent>

#include "src/shared/commands/RPZActions.h"

#include "src/shared/models/RPZAtom.h"
#include "src/shared/payloads/Payloads.h"

#include "src/ui/mapLayout/_base/LockAndVisibilityDelegate.h"

#include "src/ui/mapLayout/model/MapLayoutModel.hpp"

#include "src/shared/commands/AtomsContextualMenuHandler.h"
#include "src/shared/async-ui/progress/ProgressTracker.hpp"

class MapLayoutTree : public QTreeView, public AtomSelector {

    Q_OBJECT

    public:
        MapLayoutTree(AtomsStorage* mapMaster, QWidget* parent = nullptr);

        const QList<RPZAtom::Id> selectedIds() const override;

    protected:
        void contextMenuEvent(QContextMenuEvent *event) override;
        void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;

    private slots:
        void _handleAlterationRequest(const AlterationPayload &payload);

    private:
        AtomsContextualMenuHandler* _menuHandler = nullptr;
        AtomActionsHandler* _atomActionsHandler = nullptr;
        MapLayoutModel* _model = nullptr;
        
        void _handleHintsSignalsAndSlots();

};
