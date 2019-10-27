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

#include "src/ui/_others/ConnectivityObserver.h"

#include "src/ui/mapLayout/_base/MapLayoutAtomDelegate.h"

#include "src/ui/mapLayout/model/MapLayoutModel.hpp"

#include "src/shared/commands/AtomsContextualMenuHandler.h"
#include "src/shared/async-ui/progress/ProgressTracker.hpp"

class MapLayoutTree : public QTreeView {

    Q_OBJECT

    public:
        MapLayoutTree(AtomsStorage* mapMaster, QWidget* parent = nullptr);

    protected:
        void keyPressEvent(QKeyEvent * event) override;
        void contextMenuEvent(QContextMenuEvent *event) override;
        void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;

    private slots:
        void _handleAlterationRequest(const AlterationPayload &payload);

    private:
        void _resizeSections();

        AtomsContextualMenuHandler* _menuHandler = nullptr;
        MapLayoutModel* _model = nullptr;
        void _handleHintsSignalsAndSlots();

};