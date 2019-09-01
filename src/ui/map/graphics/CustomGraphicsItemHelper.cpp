#include "CustomGraphicsItemHelper.h"

QGraphicsItem* CustomGraphicsItemHelper::createGraphicsItem(const RPZAtom &atom, const RPZAssetMetadata &assetMetadata, bool isTemporary) {
    
    QGraphicsItem* out;

    switch(atom.type()) {
        
        case AtomType::Object:
            out = _createGenericImageBasedItem(atom, assetMetadata);
        break;
        
        case AtomType::Brush:
            out = _createBrushItem(atom, assetMetadata);
        break;

        case AtomType::Drawing:
            out = _createDrawingItem(atom);
        break;

        case AtomType::Text:
            out = _createTextItem(atom);
        break;

        default:
            return nullptr;
        break;
    }

    //update
    AtomConverter::updateGraphicsItemFromAtom(out, atom, isTemporary);        

    return out;
}

QGraphicsItem* CustomGraphicsItemHelper::createOutlineRectItem(const QPointF &scenePos) {
    
    //rect...
    QRectF rect(
        scenePos - QPointF(1,1), 
        scenePos + QPointF(1,1)
    );
    
    //pen...
    QPen pen;
    pen.setWidth(0);
    pen.setJoinStyle(Qt::MiterJoin);
    pen.setCapStyle(Qt::SquareCap);
    pen.setJoinStyle(Qt::MiterJoin);

    //create item
    auto item = new QGraphicsRectItem(rect);
    item->setPen(pen);
    item->setZValue(9999999); //hover on top
    
    return item;
}


QGraphicsRectItem* CustomGraphicsItemHelper::createMissingAssetPlaceholderItem(const RPZAtom &atom) {

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
    auto placeholder = new MapViewGraphicsRectItem(shape, pen, brush);
    
    //Update values from atom blueprint
    AtomConverter::updateGraphicsItemFromAtom(placeholder, atom);
    
    return placeholder;
}


QGraphicsItem* CustomGraphicsItemHelper::_createGenericImageBasedItem(const RPZAtom &atom, const RPZAssetMetadata &assetMetadata) {

    //get file infos
    auto pathToImageFile = assetMetadata.pathToAssetFile();
    QFileInfo pathInfo(pathToImageFile);
    
    //define graphicsitem
    QGraphicsItem* item = nullptr;
    if(pathInfo.suffix() == "svg") {
        item = new MapViewGraphicsSvgItem(pathToImageFile);
    } 
    else {
        item = new MapViewGraphicsPixmapItem(assetMetadata);
    };

    return item;
}

QGraphicsPathItem* CustomGraphicsItemHelper::_createBrushItem(const RPZAtom &atom, const RPZAssetMetadata &assetMetadata) {

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
    auto newPath = new MapViewGraphicsPathItem(shape, pen, brush);
    
    return newPath;
}

QGraphicsPathItem* CustomGraphicsItemHelper::_createDrawingItem(const RPZAtom &atom) {
    
    //define a ped
    QPen pen;
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);

    //define a default shape for ghost items
    auto shape = atom.shape();
    if(!shape.elementCount()) shape.lineTo(.01,.01);

    //create path
    auto newPath = new MapViewGraphicsPathItem(shape, pen);
    
    return newPath;
}

QGraphicsTextItem* CustomGraphicsItemHelper::_createTextItem(const RPZAtom &atom) {
    return new MapViewGraphicsTextItem(atom.text(), atom.textSize());
}
