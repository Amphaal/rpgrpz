#include "MapHint.h"

MapHint::MapHint() : _sysActor(new AlterationActor(AlterationPayload::Source::Local_System)) { 
    this->connectToAlterationEmissions();
}

void MapHint::_handleAlterationRequest(AlterationPayload &payload) { 

    ViewMapHint::_handleAlterationRequest(payload);

    //define dirty
    this->_shouldMakeMapDirty(payload);
    
}

////////////////////
// State handling //
////////////////////

QString MapHint::RPZMapFilePath() const {
    return this->_mapFilePath;
}

bool MapHint::isRemote() const {
    return this->_isRemote;
}

bool MapHint::isMapDirty() const {
    return this->_isMapDirty;
}

void MapHint::mayWantToSavePendingState(QWidget* parent, MapHint* hint) {
    
    if(!hint->isMapDirty() || hint->isRemote()) return;

    //popup
    auto result = QMessageBox::warning(
        parent, 
        hint->RPZMapFilePath(), 
        "Voulez-vous sauvegarder les modifications effectuées sur la carte ?", 
        QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes
    );

    //save state
    if(result == QMessageBox::Yes) {
        hint->saveRPZMap();
    }

}

bool MapHint::saveRPZMap() {

    if(this->_isRemote) return false;

    //save into file
    MapDatabase mapDb(this->_mapFilePath);
    auto atoms = this->atoms();
    mapDb.saveIntoFile(atoms);

    //define as clean
    this->_setMapDirtiness(false);

    return true;
}


bool MapHint::saveRPZMapAs(const QString &newFilePath) {
    if(this->_isRemote) return false;

    this->_mapFilePath = newFilePath;
    this->_mapDescriptor = QFileInfo(newFilePath).fileName();
    return this->saveRPZMap();

}

bool MapHint::loadDefaultRPZMap() {
    this->defineAsRemote();
	auto map = AppContext::getDefaultMapFile();
	return this->loadRPZMap(map);
}

bool MapHint::loadRPZMap(const QString &filePath) {
    
    if(this->_isRemote) return false;

        //tells UI that map is loading
        QMetaObject::invokeMethod(ProgressTracker::get(), "heavyAlterationStarted");

        //load file and parse it
        MapDatabase mapDb(filePath);
        this->_mapFilePath = filePath;
        this->_mapDescriptor = QFileInfo(filePath).fileName();
        this->_setMapDirtiness(false);

        //compare assets in map with assets in db 
        &&&&&&mapDb.getUsedAssetsIds() == AssetsDatabase::get()->getStoredAssetsIds();

        //create payload and queue it
        auto allAtoms = mapDb.toAtoms();
        ResetPayload payload(allAtoms);
        AlterationHandler::get()->queueAlteration(this->_sysActor, payload);

    return true;
}


bool MapHint::defineAsRemote(const QString &remoteMapDescriptor) {
    
    //define remote flag
    this->_isRemote = !remoteMapDescriptor.isEmpty();
    
    {
        //reset missing assets list
        QMutexLocker l(&this->_m_missingAssetsIdsFromDb);
        this->_missingAssetsIdsFromDb.clear();
    }

    //change map descriptor if is a remote session
    if(this->_isRemote) {
        this->_mapFilePath.clear();
        this->_mapDescriptor = remoteMapDescriptor;
    }

    //anyway, unset dirty
    this->_setMapDirtiness(false);
    
    return this->_isRemote;
}


void MapHint::_shouldMakeMapDirty(AlterationPayload &payload) {

    //if remote, never dirty
    if(this->_isRemote) return;

    //if not a network alteration type
    if(!payload.isNetworkRoutable()) return;

    //always not dirty if reset
    if(payload.type() == PayloadAlteration::PA_Reset) return;

    this->_setMapDirtiness();
}


void MapHint::_setMapDirtiness(bool dirty) {
    
    this->_isMapDirty = dirty;
    
    emit mapStateChanged(
        this->_mapDescriptor, 
        this->_isMapDirty
    );

}


////////////////////////
// END State handling //
////////////////////////