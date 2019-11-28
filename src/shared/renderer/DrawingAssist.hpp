#pragma once

#include "src/shared/hints/HintThread.hpp"
#include <QGraphicsView>

#include "src/shared/renderer/graphics/MapViewGraphics.h"

class DrawingAssist {
    public:
        using IsCommitedDrawing = bool;

        DrawingAssist(QGraphicsView* view) : _view(view) { }

        void addDrawingPoint(const QPoint &cursorPosInWindow) {
            
            auto scenePos = this->_view->mapToScene(cursorPosInWindow);

            if(this->_stickyBrushIsDrawing) {
                this->_savePosAsStickyNode(scenePos, cursorPosInWindow);
            } 
            
            else {
                this->_beginDrawing(scenePos);
            }

        }

        void updateDrawingPath(const QPoint &evtPoint, const RPZAtom::Type &type) {
            
            //if no temp, stop
            if(!this->_tempDrawing) return;

            //get existing path
            auto existingPath = this->_tempDrawing->path();

            //define destination coordonate
            auto sceneCoord = this->_view->mapToScene(evtPoint);
            auto pathCoord = this->_tempDrawing->mapFromScene(sceneCoord);

            switch(type) {
                
                case RPZAtom::Type::Drawing:
                    existingPath.lineTo(pathCoord);
                break;

                case RPZAtom::Type::Brush:
                    this->_updateDrawingPathForBrush(pathCoord, existingPath, this->_tempDrawing);
                break;

                default:
                    break;

            }

            //save as new path
            this->_tempDrawing->setPath(existingPath);

        }

        void onMouseRelease() {
            if(!this->_stickyBrushIsDrawing) this->mayCommitDrawing();
        }

        void mayCommitDrawing() {
            
            //if no temporary drawing, stop
            if(!this->_tempDrawing) return;
            
            //if too small stop
            auto path = this->_tempDrawing->path();
            if(path.elementCount() < 2) return;

            //add definitive path
            this->_commitedDrawingId = HintThread::hint()->integrateGraphicsItemAsPayload(this->_tempDrawing);

        }
        
        IsCommitedDrawing compareItemToCommitedDrawing(QGraphicsItem* itemInserted) {
            
            if(!this->_commitedDrawingId) return false;
            
            if(HintThread::hint()->getAtomIdFromGraphicsItem(itemInserted) != this->_commitedDrawingId) return false;
            
            this->_destroyTempDrawing();

            this->_commitedDrawingId = 0;

            return true;

        }

        void clearDrawing() {
            this->_destroyTempDrawing();
            this->_commitedDrawingId = 0;
        }

    private:
        QGraphicsView* _view = nullptr;
        RPZAtom::Id _commitedDrawingId = 0;

        //drawing...
        MapViewDrawing* _tempDrawing = nullptr;
        QList<QGraphicsItem*> _tempDrawingHelpers;
        bool _stickyBrushIsDrawing = false;
        int _stickyBrushValidNodeCount = 0;

        void _destroyTempDrawing() {
            if(!this->_tempDrawing) return;
            
            //remove helpers
            for(const auto helper : this->_tempDrawingHelpers) {
                delete helper;
            }
            this->_tempDrawingHelpers.clear();
                
            //remove drawing
            delete this->_tempDrawing;
            this->_tempDrawing = nullptr;

            //reset sticky
            if(this->_stickyBrushIsDrawing) {   
                this->_stickyBrushIsDrawing = false;
                this->_stickyBrushValidNodeCount = 0;
            }
            
        }

        void _beginDrawing(const QPointF &scenePos) {

            //destroy temp
            this->_destroyTempDrawing();

            //create item 
            this->_tempDrawing = static_cast<MapViewDrawing*>(HintThread::hint()->generateGraphicsFromTemplate());
                
                //define pos
                auto centerScenePos = scenePos;
                centerScenePos = centerScenePos - this->_tempDrawing->boundingRect().center();
                this->_tempDrawing->setPos(centerScenePos);

            //add to scene
            this->_view->scene()->addItem(this->_tempDrawing);

            //determine if it must be sticky
            this->_stickyBrushIsDrawing = HintThread::hint()->templateAtom().brushType() == RPZAtom::BrushType::Cutter;
            this->_stickyBrushValidNodeCount = this->_stickyBrushIsDrawing ? this->_tempDrawing->path().elementCount() : 0;

            //add outline if sticky
            if(this->_stickyBrushIsDrawing) {
                this->_addOutlineRect(scenePos);
            }

        }

        void _updateDrawingPathForBrush(const QPointF &pathCoord, QPainterPath &pathToAlter, MapViewGraphicsPathItem* sourceTemplate) {
            
            switch(HintThread::hint()->templateAtom().brushType()) {
                
                case RPZAtom::BrushType::Stamp: {
                    
                    //if contained in path, nothing to do
                    if(pathToAlter.contains(pathCoord)) return;

                    auto xRatio = qCeil(pathCoord.x() / sourceTemplate->sourceBrushSize().width()) - 1;
                    auto yRatio = qCeil(pathCoord.y() / sourceTemplate->sourceBrushSize().height()) - 1;
                    auto expectedStampPos = QPointF(
                        xRatio * sourceTemplate->sourceBrushSize().width(),
                        yRatio * sourceTemplate->sourceBrushSize().height()
                    );

                    //if something at expected, nothing to do
                    if(pathToAlter.contains(expectedStampPos)) return;
                    
                    //add rect
                    QRectF rect(expectedStampPos, sourceTemplate->sourceBrushSize());
                    pathToAlter.addRect(rect);

                }
                break;

                case RPZAtom::BrushType::Cutter: {
                    auto count = pathToAlter.elementCount();
                    
                    //if no temporary node, create it
                    if(this->_stickyBrushValidNodeCount == count) {
                        pathToAlter.lineTo(pathCoord);
                    } 
                    
                    //update temporary node
                    else {
                        pathToAlter.setElementPositionAt(count-1, pathCoord.x(), pathCoord.y());
                    }
                }                
                break;

                case RPZAtom::BrushType::Ovale: {
                    pathToAlter = QPainterPath();
                    QRectF rect(QPointF(0,0), pathCoord);
                    pathToAlter.addEllipse(rect);
                }
                break;

                case RPZAtom::BrushType::Rectangle: {
                    pathToAlter = QPainterPath();
                    QRectF rect(QPointF(0,0), pathCoord);
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
            
            //dest pos
            auto destCoord = this->_tempDrawing->mapFromScene(scenePos);
            
            //Update
            auto path = this->_tempDrawing->path();
            auto count = path.elementCount();
            path.setElementPositionAt(count-1, destCoord.x(), destCoord.y());
            this->_tempDrawing->setPath(path);
            
            //add visual helper
            this->_addOutlineRect(scenePos);

            //update
            this->_stickyBrushValidNodeCount = count;
            
            //check vicinity between last node and first node
            auto firstNode = this->_tempDrawing->mapToScene(path.elementAt(0));
            auto firstNodeGlobalPos = this->_view->mapFromScene(firstNode);
            auto gap = cursorPosInWindow - firstNodeGlobalPos;

            //if gap is minimal, understand that the users wants to end the drawing
            if(gap.manhattanLength() < 20) {
                this->mayCommitDrawing();
            }
        }

};