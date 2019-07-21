#include "MapViewGraphicsScene.h"

MapViewGraphicsScene::MapViewGraphicsScene(int defaultSize) : QGraphicsScene(defaultSize, defaultSize, defaultSize, defaultSize) { }

void MapViewGraphicsScene::onGraphicsItemCustomChange(QGraphicsItem* item, MapViewCustomItemsEventFlag flag) {
    emit sceneItemChanged(item, (int)flag);
};

QGraphicsItem* MapViewGraphicsScene::addToScene(RPZAtom &atom, AssetMetadata &assetMetadata, bool isTemporary) {
    
    QGraphicsItem* out;

    switch(atom.type()) {
        
        case AtomType::Object:
            out = this->_addGenericImageBasedItem(atom, assetMetadata);
        break;
        
        case AtomType::Brush:
            out = this->_addBrush(atom, assetMetadata);
        break;

        case AtomType::Drawing:
            out = this->_addDrawing(atom);
        break;

        case AtomType::Text:
            out = this->_addText(atom);
        break;

        default:
            return nullptr;
        break;
    }

    //update
    AtomConverter::updateGraphicsItemFromAtom(out, atom, isTemporary);        

    //add atomType tracker
    this->addItem(out);

    //prevent notifications on move to kick in for temporary items
    if(auto notifier = dynamic_cast<MapViewItemsNotifier*>(out)) {
        if(isTemporary) {
            notifier->disableNotifications();
        }
        else {
            notifier->activateNotifications();
        }
    }

    return out;
}

QGraphicsItem* MapViewGraphicsScene::addOutlineRect(QPointF &scenePos) {
    
    //rect...
    QRectF rect(scenePos - QPointF(1,1), scenePos + QPointF(1,1));
    
    //pen...
    QPen pen;
    pen.setWidth(0);
    pen.setJoinStyle(Qt::MiterJoin);
    pen.setCapStyle(Qt::SquareCap);
    pen.setJoinStyle(Qt::MiterJoin);

    //add item
    auto item = this->addRect(rect, pen);

    //hover on top
    item->setZValue(9999999);
    
    return item;
}


QGraphicsRectItem* MapViewGraphicsScene::addMissingAssetPH(RPZAtom &atom) {

    //pen to draw the rect with
    QPen pen;
    pen.setStyle(Qt::DashLine);
    pen.setJoinStyle(Qt::MiterJoin);
    pen.setColor(Qt::GlobalColor::red);
    pen.setWidth(0);

    //background brush
    QBrush brush(QColor(255, 0, 0, 128));

    //shape
    auto shape = atom.shape().boundingRect();

    //create graphics item
    auto placeholder = new MapViewGraphicsRectItem(this, shape, pen, brush);
    
    //Update values from atom blueprint
    AtomConverter::updateGraphicsItemFromAtom(placeholder, atom);
    
    //add to scene
    this->addItem(placeholder);

    //activate notifications
    if(auto notifier = dynamic_cast<MapViewItemsNotifier*>(placeholder)) {
        notifier->activateNotifications();
    }

    return placeholder;
}


QGraphicsItem* MapViewGraphicsScene::_addGenericImageBasedItem(RPZAtom &atom, AssetMetadata &assetMetadata) {

    //get file infos
    auto pathToImageFile = assetMetadata.pathToAssetFile();
    QFileInfo pathInfo(pathToImageFile);
    
    //define graphicsitem
    QGraphicsItem* item = nullptr;
    if(pathInfo.suffix() == "svg") {
        item = new MapViewGraphicsSvgItem(this, pathToImageFile);
    } 
    else {
        item = new MapViewGraphicsPixmapItem(this, assetMetadata);
    };

    return item;
}

QGraphicsPathItem* MapViewGraphicsScene::_addBrush(RPZAtom &atom, AssetMetadata &assetMetadata) {

    //define a ped
    QPen pen;
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setColor(QColor(255, 255, 255, 0));

    //define a default shape for ghost items
    auto shape = atom.shape();
    shape.setFillRule(Qt::FillRule::WindingFill);

    //configure brush
    QBrush brush;
    
        //get texture from cache
        QPixmap cached;
        auto assetId = assetMetadata.assetId();
        auto found = QPixmapCache::find(assetId, &cached);
        if(!found) {
            cached = QPixmap(assetMetadata.pathToAssetFile());
            QPixmapCache::insert(assetId, cached);
        }
        brush.setTexture(cached);
    
    //create path
    auto newPath = new MapViewGraphicsPathItem(this, shape, pen, brush);
    
    return newPath;
}

QGraphicsPathItem* MapViewGraphicsScene::_addDrawing(RPZAtom &atom) {
    
    //define a ped
    QPen pen;
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);

    //define a default shape for ghost items
    auto shape = atom.shape();
    if(!shape.elementCount()) shape.lineTo(.01,.01);

    //create path
    auto newPath = new MapViewGraphicsPathItem(this, shape, pen);
    
    return newPath;
}

QGraphicsTextItem* MapViewGraphicsScene::_addText(RPZAtom &atom) {
    return new MapViewGraphicsTextItem(this, atom.text(), atom.textSize());
}
