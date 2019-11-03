#include "AtomRenderer.h"

QGraphicsItem* AtomRenderer::createGraphicsItem(const RPZAtom &atom, const RPZAsset &asset, const QSizeF &tileSize, bool isTemporary) {
    
    QGraphicsItem* out;
    auto type = atom.type();

    switch(type) {
        
        case RPZAtom::Type::Object:
            out = _createGenericImageBasedItem(atom, asset);
        break;
        
        case RPZAtom::Type::Brush:
            out = _createBrushItem(atom, asset);
        break;

        case RPZAtom::Type::Drawing:
            out = _createDrawingItem(atom);
        break;

        case RPZAtom::Type::Text:
            out = _createTextItem(atom);
        break;

        case RPZAtom::Type::Event:
            out = _createUnscalableToken(atom);
        break;

        case RPZAtom::Type::Player:
            out = _createPlayerToken(atom, tileSize);
        break;

        default: {
            qWarning() << "Map : Cannot create QGraphicsItem associated with atom, because atomType have no handler associated !";
            return nullptr;
        }
        break;
        
    }

    //update
    AtomConverter::updateGraphicsItemFromAtom(out, atom, isTemporary);        

    //if base is QGraphicsSvgItem, QGraphicsTextItem, and QGraphicsWidget, move handler to main GUI thread
    if(auto signalHandler = out->toGraphicsObject()) {
        signalHandler->moveToThread(QApplication::instance()->thread());
    }

    return out;
}

QGraphicsItem* AtomRenderer::createOutlineRectItem(const QPointF &scenePos) {
    
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
    item->setZValue(AppContext::TOP_Z_INDEX); //hover on top
    
    return item;
}


QGraphicsRectItem* AtomRenderer::createMissingAssetPlaceholderItem(const RPZAtom &atom) {

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


QGraphicsItem* AtomRenderer::_createGenericImageBasedItem(const RPZAtom &atom, const RPZAsset &asset) {

    //get file infos
    auto filepathToAsset = asset.filepath();
    
    //define graphicsitem
    QGraphicsItem* item = nullptr;
    if(asset.fileExtension() == "svg") {
        item = new MapViewGraphicsSvgItem(filepathToAsset);
    } 
    else {
        item = new MapViewGraphicsPixmapItem(asset);
    };

    return item;
}

QGraphicsPathItem* AtomRenderer::_createBrushItem(const RPZAtom &atom, const RPZAsset &asset) {

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
    brush.setTexture(asset.filepath());
    
    //create path
    auto newPath = new MapViewGraphicsPathItem(shape, pen, brush);
    
    return newPath;
}

QGraphicsPathItem* AtomRenderer::_createDrawingItem(const RPZAtom &atom) {
    
    //define a pen
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

QGraphicsTextItem* AtomRenderer::_createTextItem(const RPZAtom &atom) {
    return new MapViewGraphicsTextItem(atom.text(), atom.textSize());
}

QGraphicsPixmapItem* AtomRenderer::_createUnscalableToken(const RPZAtom &atom) {
    
    auto pathToIcon = RPZAtom::iconPathByAtomType.value(atom.type());
    auto out = new MapViewGraphicsPixmapItem(pathToIcon);
    out->setFlag(QGraphicsItem::GraphicsItemFlag::ItemIgnoresTransformations, true);

    return out;

}

MapViewToken* AtomRenderer::_createPlayerToken(const RPZAtom &atom, const QSizeF &tileSize) {
    auto out = new MapViewToken(tileSize, atom.defaultColor());
    return out;
}