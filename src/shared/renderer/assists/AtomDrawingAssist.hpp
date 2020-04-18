// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical or audio resources available within the source code may
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GPL.

#pragma once

#include <QGraphicsView>

#include "src/shared/hints/HintThread.hpp"

#include "src/shared/renderer/graphics/MapViewGraphics.h"

#include "src/helpers/VectorSimplifier.hpp"

class AtomDrawingAssist {
 public:
    using IsCommitedDrawing = bool;

    explicit AtomDrawingAssist(QGraphicsView* view) : _view(view) { }

    void addDrawingPoint(const QPoint &cursorPosInWindow, const RPZAtom &drawnFrom) {
        auto scenePos = this->_view->mapToScene(cursorPosInWindow);

        this->_drawnType = drawnFrom.type();
        this->_brushType = drawnFrom.brushType();
        this->_stickyBrushIsDrawing = this->_drawnType == RPZAtom::Type::Brush && this->_brushType == RPZAtom::BrushType::Cutter;

        if (this->_stickyBrushIsDrawing) {
            if (!this->_tempDrawing) this->_beginDrawing(scenePos);
            else
                this->_savePosAsStickyNode(scenePos, cursorPosInWindow);
        } else {
            this->_destroyTempDrawing();  // destroy previous
            this->_beginDrawing(scenePos);
        }
    }

    void updateDrawingPath(const QPoint &evtPoint) {
        // if no temp, stop
        if (!this->_tempDrawing) return;

        // get existing path
        auto existingPath = this->_tempDrawing->path();

        // define destination coordonate
        auto sceneCoord = this->_view->mapToScene(evtPoint);
        auto pathCoord = this->_tempDrawing->mapFromScene(sceneCoord);

        switch (this->_drawnType) {
            case RPZAtom::Type::Drawing:
                existingPath.lineTo(pathCoord);
            break;

            case RPZAtom::Type::Brush:
                this->_updateDrawingPathForBrush(pathCoord, existingPath, this->_tempDrawing);
            break;

            default:
                break;
        }

        // save as new path
        this->_tempDrawing->setPath(existingPath);
    }

    void onMouseRelease() {
        if (!this->_stickyBrushIsDrawing) this->mayCommitDrawing();
    }

    void mayCommitDrawing() {
        // if no temporary drawing, stop
        if (!this->_tempDrawing) return;

        // if too small stop
        auto path = this->_tempDrawing->path();
        if (path.elementCount() < 2) return;

        // Reduce size
        this->_mightReduceItemPath(path, this->_tempDrawing);

        // add definitive path
        this->_commitedDrawingId = HintThread::hint()->integrateGraphicsItemAsPayload(this->_tempDrawing);
    }

    IsCommitedDrawing compareItemToCommitedDrawing(QGraphicsItem* itemInserted) {
        if (!this->_commitedDrawingId) return false;
        if (HintThread::hint()->getAtomIdFromGraphicsItem(itemInserted) != this->_commitedDrawingId) return false;

        this->clearDrawing();
        return true;
    }

    void clearDrawing() {
        this->_destroyTempDrawing();
        this->_commitedDrawingId = 0;
        this->_drawnType = (RPZAtom::Type)0;
        this->_brushType = (RPZAtom::BrushType)0;
    }

 private:
    QGraphicsView* _view = nullptr;
    RPZAtom::Id _commitedDrawingId = 0;

    // drawing...
    MapViewGraphicsPathItem* _tempDrawing = nullptr;
    QList<QGraphicsItem*> _tempDrawingHelpers;
    bool _stickyBrushIsDrawing = false;
    int _stickyBrushValidNodeCount = 0;
    RPZAtom::Type _drawnType = (RPZAtom::Type)0;
    RPZAtom::BrushType _brushType = (RPZAtom::BrushType)0;

    void _destroyTempDrawing() {
        if (!this->_tempDrawing) return;

        // remove helpers
        for (const auto helper : this->_tempDrawingHelpers) {
            delete helper;
        }
        this->_tempDrawingHelpers.clear();

        // remove drawing
        delete this->_tempDrawing;
        this->_tempDrawing = nullptr;

        // reset sticky
        if (this->_stickyBrushIsDrawing) {
            this->_stickyBrushIsDrawing = false;
            this->_stickyBrushValidNodeCount = 0;
        }
    }

    void _beginDrawing(const QPointF &scenePos) {
        // create item
        this->_tempDrawing = static_cast<MapViewDrawing*>(HintThread::hint()->generateGraphicsFromTemplate());

            // define pos
            auto centerScenePos = scenePos;
            centerScenePos = centerScenePos - this->_tempDrawing->boundingRect().center();
            this->_tempDrawing->setPos(centerScenePos);

        // add to scene
        this->_view->scene()->addItem(this->_tempDrawing);

        // determine if it must be sticky
        this->_stickyBrushValidNodeCount = this->_stickyBrushIsDrawing ? this->_tempDrawing->path().elementCount() : 0;

        // add outline if sticky
        if (this->_stickyBrushIsDrawing) {
            this->_addOutlineRect(scenePos);
        }
    }

    void _updateDrawingPathForBrush(const QPointF &pathCoord, QPainterPath &pathToAlter, MapViewGraphicsPathItem* sourceTemplate) {
        switch (HintThread::hint()->templateAtom().brushType()) {
            case RPZAtom::BrushType::Stamp: {
                // if contained in path, nothing to do
                if (pathToAlter.contains(pathCoord)) return;

                auto xRatio = qCeil(pathCoord.x() / sourceTemplate->sourceBrushSize().width()) - 1;
                auto yRatio = qCeil(pathCoord.y() / sourceTemplate->sourceBrushSize().height()) - 1;
                auto expectedStampPos = QPointF(
                    xRatio * sourceTemplate->sourceBrushSize().width(),
                    yRatio * sourceTemplate->sourceBrushSize().height()
                );

                // if something at expected, nothing to do
                if (pathToAlter.contains(expectedStampPos)) return;

                // add rect
                QRectF rect(expectedStampPos, sourceTemplate->sourceBrushSize());
                pathToAlter.addRect(rect);
            }
            break;

            case RPZAtom::BrushType::Cutter: {
                auto count = pathToAlter.elementCount();

                // if no temporary node, create it
                if (this->_stickyBrushValidNodeCount == count) {
                    pathToAlter.lineTo(pathCoord);
                } else {  // update temporary node
                    pathToAlter.setElementPositionAt(count-1, pathCoord.x(), pathCoord.y());
                }
            }
            break;

            case RPZAtom::BrushType::Ovale: {
                pathToAlter = QPainterPath();
                QRectF rect(QPointF(0, 0), pathCoord);
                pathToAlter.addEllipse(rect);
            }
            break;

            case RPZAtom::BrushType::Rectangle: {
                pathToAlter = QPainterPath();
                QRectF rect(QPointF(0, 0), pathCoord);
                pathToAlter.addRect(rect);
            }
            break;

            case RPZAtom::BrushType::Scissors: {
                pathToAlter.lineTo(pathCoord);
            }
            break;

            case RPZAtom::BrushType::RoundBrush: {
                pathToAlter.lineTo(pathCoord);
            }
            break;

            default:
                break;
        }
    }

    void _addOutlineRect(const QPointF &scenePos) {
        auto outline = AtomRenderer::createOutlineRectItem(scenePos);
        this->_view->scene()->addItem(outline);
        this->_tempDrawingHelpers.append(outline);
    }

    void _savePosAsStickyNode(const QPointF &scenePos, const QPoint &cursorPosInWindow) {
        // dest pos
        auto destCoord = this->_tempDrawing->mapFromScene(scenePos);

        // Update
        auto path = this->_tempDrawing->path();
        auto count = path.elementCount();
        path.setElementPositionAt(count-1, destCoord.x(), destCoord.y());
        this->_tempDrawing->setPath(path);

        // add visual helper
        this->_addOutlineRect(scenePos);

        // update
        this->_stickyBrushValidNodeCount = count;

        // check vicinity between last node and first node
        auto firstNode = this->_tempDrawing->mapToScene(path.elementAt(0));
        auto firstNodeGlobalPos = this->_view->mapFromScene(firstNode);
        auto gap = cursorPosInWindow - firstNodeGlobalPos;

        // if gap is minimal, understand that the users wants to end the drawing
        if (gap.manhattanLength() < 20) {
            this->mayCommitDrawing();
        }
    }

    void _mightReduceItemPath(QPainterPath sourcePath, MapViewGraphicsPathItem* item) {
        if (!this->_mustBeSimplified()) return;

        sourcePath = VectorSimplifier::reducePath(sourcePath);

        item->setPath(sourcePath);
    }

    bool _mustBeSimplified() {
        if (this->_drawnType == RPZAtom::Type::Drawing) return true;
        if (this->_drawnType != RPZAtom::Type::Brush) return false;
        if (this->_brushType == RPZAtom::BrushType::RoundBrush || this->_brushType == RPZAtom::BrushType::Scissors) return true;
        return false;
    }
};
