#include "MapHint.h"

MapHint::MapHint(QGraphicsView* boundGv) : ViewMapHint(boundGv) { }

void MapHint::_handlePayload(AlterationPayload &payload) { 

    ViewMapHint::_handlePayload(payload);

    //define dirty
    this->_shouldMakeDirty(payload);
}

////////////////////
// State handling //
////////////////////

QString MapHint::stateFilePath() {
    return this->_stateFilePath;
}

bool MapHint::isRemote() {
    return this->_isRemote;
}

bool MapHint::isDirty() {
    return this->_isDirty;
}

void MapHint::mayWantToSavePendingState() {
    if(!this->_isDirty || this->_isRemote) return;

    //popup
    auto result = QMessageBox::warning(
        this->_boundGv, 
        this->_stateFilePath, 
        "Voulez-vous sauvegarder les modifications effectuées sur la carte ?", 
        QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes
    );

    //save state
    if(result == QMessageBox::Yes) {
        this->saveState();
    }

}

bool MapHint::saveState() {

    if(this->_isRemote) return false;

    //save into file
    MapDatabase mapDb(this->_stateFilePath);
    mapDb.saveIntoFile(this->_atomsById);

    //define as clean
    this->_setDirty(false);

    return true;
}


bool MapHint::saveStateAs(const QString &newFilePath) {
    if(this->_isRemote) return false;

    this->_stateFilePath = newFilePath;
    return this->saveState();

}

bool MapHint::loadDefaultState() {
    return this->loadState(
        AppContext::getDefaultMapFile()
    );
}


bool MapHint::loadState(const QString &filePath) {
    
    if(this->_isRemote) return false;

    //ask for save if dirty before loading
    this->mayWantToSavePendingState();

    //load file and parse it
    MapDatabase mapDb(filePath);
    auto allAtoms = mapDb.toAtoms();
    auto payload = ResetPayload(allAtoms);
    this->_handlePayload(payload);
    
    //change file path and define as clean
    this->_stateFilePath = filePath;
    this->_setDirty(false);

    return true;
}


bool MapHint::defineAsRemote(const QString &remoteMapDescriptor) {
    
    //define remote flag
    this->_isRemote = !remoteMapDescriptor.isEmpty();
    
    //reset missing assets list
    this->_missingAssetsIdsFromDb.clear();

    //change map descriptor if is a remote session
    if(this->_isRemote) this->_stateFilePath = remoteMapDescriptor;

    //anyway, unset dirty
    this->_setDirty(false);
    
    return this->_isRemote;
}


void MapHint::_shouldMakeDirty(AlterationPayload &payload) {
    
    //if remote, never dirty
    if(this->_isRemote) return;

    //if not a network alteration type
    if(!payload.isNetworkRoutable()) return;

    this->_setDirty();
}


void MapHint::_setDirty(bool dirty) {
    this->_isDirty = dirty;
    emit mapFileStateChanged(this->_stateFilePath, this->_isDirty);
}


////////////////////////
// END State handling //
////////////////////////
