#include "MapHint.h"

MapHint::MapHint(QGraphicsView* boundGv) : ViewMapHint(boundGv) { }

void MapHint::_handlePayload(AlterationPayload &payload) { 

    ViewMapHint::_handlePayload(payload);

    //define dirty
    this->_shouldMakeMapDirty(payload);
}

////////////////////
// State handling //
////////////////////

QString MapHint::mapFilePath() {
    return this->_mapFilePath;
}

bool MapHint::isRemote() {
    return this->_isRemote;
}

bool MapHint::isMapDirty() {
    return this->_isMapDirty;
}

void MapHint::mayWantToSavePendingState() {
    if(!this->_isMapDirty || this->_isRemote) return;

    //popup
    auto result = QMessageBox::warning(
        this->_boundGv, 
        this->_mapFilePath, 
        "Voulez-vous sauvegarder les modifications effectuées sur la carte ?", 
        QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes
    );

    //save state
    if(result == QMessageBox::Yes) {
        this->saveMap();
    }

}

bool MapHint::saveMap() {

    if(this->_isRemote) return false;

    //save into file
    MapDatabase mapDb(this->_mapFilePath);
    mapDb.saveIntoFile(this->_atomsById);

    //define as clean
    this->_setMapDirtiness(false);

    return true;
}


bool MapHint::saveMapAs(const QString &newFilePath) {
    if(this->_isRemote) return false;

    this->_mapFilePath = newFilePath;
    return this->saveMap();

}

bool MapHint::loadDefaultMap() {
    return this->loadMap(
        AppContext::getDefaultMapFile()
    );
}


bool MapHint::loadMap(const QString &filePath) {
    
    if(this->_isRemote) return false;

    //ask for save if dirty before loading
    this->mayWantToSavePendingState();

    //loader....
    this->_boundGv->setForegroundBrush(*this->_hiddingBrush);

        //load file and parse it
        MapDatabase mapDb(filePath);
        auto allAtoms = mapDb.toAtoms();
        auto payload = ResetPayload(allAtoms);
        this->_handlePayload(payload);
        
        //change file path and define as clean
        this->_mapFilePath = filePath;
        this->_setMapDirtiness(false);

    //loader...
    this->_boundGv->setForegroundBrush(QBrush());

    return true;
}


bool MapHint::defineAsRemote(const QString &remoteMapDescriptor) {
    
    //define remote flag
    this->_isRemote = !remoteMapDescriptor.isEmpty();
    
    //reset missing assets list
    this->_missingAssetsIdsFromDb.clear();

    //change map descriptor if is a remote session
    if(this->_isRemote) this->_mapFilePath = remoteMapDescriptor;

    //anyway, unset dirty
    this->_setMapDirtiness(false);
    
    return this->_isRemote;
}


void MapHint::_shouldMakeMapDirty(AlterationPayload &payload) {
    
    //if remote, never dirty
    if(this->_isRemote) return;

    //if not a network alteration type
    if(!payload.isNetworkRoutable()) return;

    this->_setMapDirtiness();
}


void MapHint::_setMapDirtiness(bool dirty) {
    this->_isMapDirty = dirty;
    emit mapFileStateChanged(this->_mapFilePath, this->_isMapDirty);
}


////////////////////////
// END State handling //
////////////////////////