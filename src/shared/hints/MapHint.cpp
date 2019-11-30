#include "MapHint.h"

MapHint::MapHint() : _sysActor(new AlterationActor(Payload::Source::Local_System)) { 
    this->connectToAlterationEmissions();
}

void MapHint::_handleAlterationRequest(const AlterationPayload &payload) { 

    ViewMapHint::_handleAlterationRequest(payload);

    //define dirty
    this->_shouldMakeMapDirty(payload);
    
}

////////////////////
// State handling //
////////////////////

bool MapHint::isRemote() const {
    return this->_isRemote;
}

bool MapHint::isMapDirty() const {
    return this->_isMapDirty;
}

const QString MapHint::mapFilePath() const {
    auto mapPath = this->map().dbFilePath();
    return mapPath.isEmpty() ? AppContext::getDefaultMapFilePath() : mapPath;
}

void MapHint::mayWantToSavePendingState(QWidget* parent, MapHint* hint) {
    
    if(!hint->isMapDirty() || hint->isRemote()) return;

    //popup
    auto result = QMessageBox::warning(
        parent, 
        hint->map().dbFilePath(), 
        tr("Do you want to save changes done to this map ?"), 
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
    this->map().save();

    //define as clean
    this->_setMapDirtiness(false);

    return true;
}

bool MapHint::createNewRPZMapAs(const QString &newFilePath) {
    
    //reject request if remoted
    if(this->_isRemote) return false;

    //define descr
    this->_mapDescriptor = QFileInfo(newFilePath).fileName();

    //load
    return this->loadRPZMap(newFilePath);

}

bool MapHint::saveRPZMapAs(const QString &newFilePath) {
    
    //reject request if remoted
    if(this->_isRemote) return false;

    //define descr
    this->_mapDescriptor = QFileInfo(newFilePath).fileName();
    this->map().changeSourceFile(newFilePath);
    
    //save
    return this->saveRPZMap();

}

bool MapHint::loadDefaultRPZMap() {
    auto fp = this->mapFilePath();
    this->ackRemoteness(fp);
	return this->loadRPZMap(fp);
}

bool MapHint::loadRPZMap(const QString &filePath) {
    
    if(this->_isRemote) return false;

        qDebug() << qUtf8Printable(QStringLiteral(u"Loading map \"%1\"...").arg(filePath));

        //tells UI that map is loading
        QMetaObject::invokeMethod(ProgressTracker::get(), "heavyAlterationStarted");

        //load file and parse it
        this->_mapDescriptor = QFileInfo(filePath).fileName();
        this->_setMapDirtiness(false);

        //fill database
        this->_replaceMap(MapDatabase(filePath));

        //create payload and queue it
        ResetPayload payload(this->map());
        AlterationHandler::get()->queueAlteration(this->_sysActor, payload);

    return true;
}

bool  MapHint::ackRemoteness(const QString &tblMapFilePath) {
    
    this->_isRemote = false;
    this->_mapDescriptor = tblMapFilePath;

    return this->_ackRemoteness();

}

bool MapHint::ackRemoteness(const RPZUser &connectedUser, const QString &remoteAddress) {
    
    //define remote flag
    this->_isRemote = connectedUser.role() != RPZUser::Role::Host;

    //change map descriptor if is a remote session
    if(this->_isRemote) this->_mapDescriptor = remoteAddress;

    return this->_ackRemoteness();

}

bool MapHint::_ackRemoteness() {
    
    //anyway, unset dirty
    this->_setMapDirtiness(false);
    
    //return remoteness
    return this->_isRemote;

}

void MapHint::_shouldMakeMapDirty(const AlterationPayload &payload) {

    //if remote, never dirty
    if(this->_isRemote) return;

    //if not a network alteration type
    if(!payload.isNetworkRoutable()) return;

    //ResetPayload is dirty if local hardset of map parameters 
    if(auto mPayload = dynamic_cast<const ResetPayload*>(&payload)) {
        auto mapParamsUpdate = mPayload->isFromMapParametersUpdate();
        if(!mapParamsUpdate) return;
    }

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