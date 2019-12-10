#include "JSONDatabase.h"

JSONDatabase::JSONDatabase(const QString &logId) : _logId(logId) {}

void JSONDatabase::log(const QString &msg) {
    qDebug() << qUtf8Printable(this->_logId) << ":" << qUtf8Printable(msg);
}

void JSONDatabase::changeSourceFile(const QString &newSource) {
    
    if(!_destfile) return;
    delete this->_destfile;

    this->_destfile = new QFile(newSource);

}

void JSONDatabase::_initDatabaseFromJSONFile(const QString &dbFilePath) {
    
    //read database file as JSON
    if(this->_destfile) delete this->_destfile;
    this->_destfile = new QFile(dbFilePath);

    //if file is empty or doesnt exist
    if(!this->_destfile->size() || !this->_destfile->exists()) {
        this->_updateDbFile(this->_emptyDbFile());
    }

    //try to read the file
    auto document = this->_readAsDocument();

    //corrupted file, move it and create a new one
    if(document.isNull()) {
        this->log("Cannot read database, creating a new one...");
        this->_duplicateDbFile("error");
        this->_updateDbFile(this->_emptyDbFile());
        return;
    }

    //compare versions
    auto dbCopy = document.object();
    auto currentVersion = this->_getDbVersion(dbCopy);
    auto expectedVersion = this->apiVersion();
    
    //handle missmatch and update temporary database accordingly
    if(expectedVersion != currentVersion) {       
        auto error = this->_handleVersionMissmatch(dbCopy, currentVersion);
        if(error) dbCopy = this->_emptyDbFile();
    }

    this->_setupFromDbCopy(dbCopy);

}

void JSONDatabase::_setupFromDbCopy(const QJsonObject &copy) {
    this->_dbCopy = copy;
    this->_setupLocalData();
}

const QString JSONDatabase::dbFilePath() const {
    if(!this->_destfile) return QString();
    return this->_destfile->fileName();
}

const JSONDatabase::Version JSONDatabase::dbVersion() {
    return _getDbVersion(this->_dbCopy);
}

const QJsonObject& JSONDatabase::db() {
    return this->_dbCopy;
}

QJsonObject JSONDatabase::entityAsObject(const QString &entityKey) {
    return this->db().value(entityKey).toObject();
}

QJsonArray JSONDatabase::entityAsArray(const QString &entityKey) {
    return this->db().value(entityKey).toArray();
}

JSONDatabase::Version JSONDatabase::_getDbVersion(const QJsonObject &db) {
    auto version = db.value(QStringLiteral(u"version")).toInt();
    return version;
}

void JSONDatabase::updateFrom(QJsonObject &base, const QString &entityKey, const QVariantMap &entity) {
    base.insert(entityKey, QJsonObject::fromVariantMap(entity));
}

void JSONDatabase::updateFrom(QJsonObject &base, const QString &entityKey, const QSet<QString> &entity) {
    base.insert(entityKey, QJsonArray::fromStringList(entity.toList()));
}

void JSONDatabase::updateFrom(QJsonObject &base, const QString &entityKey, const QVariantList &entity) {
    base.insert(entityKey, QJsonArray::fromVariantList(entity));
}

void JSONDatabase::updateFrom(QJsonObject &base, const QString &entityKey, const QVariantHash &entity) {
    base.insert(entityKey, QJsonObject::fromVariantHash(entity));
}

QHash<JSONDatabase::Version, JSONDatabase::UpdateHandler> JSONDatabase::_getUpdateHandlers() {
    return QHash<JSONDatabase::Version, JSONDatabase::UpdateHandler>();
}

bool JSONDatabase::_handleVersionMissmatch(QJsonObject &databaseToUpdate, JSONDatabase::Version databaseToUpdateVersion) {
        
    this->log("Database does not match API version !");

    //duplicate file to migrate
    #ifndef NDEBUG
        this->_duplicateDbFile("oldVersion");
    #endif

    auto defaultBehavior = [&](const QString &reason){
        
        this->log(QStringLiteral(u"Database have not been updated : %1").arg(reason));
        
        #ifndef NDEBUG
            this->_updateDbFile(this->_emptyDbFile());
            this->log("Empty database created !");
        #endif

        return true;

    };

    //get handlers
    auto handlers = this->_getUpdateHandlers();

    //if no handlers
    if(!handlers.count()) return defaultBehavior("No handlers found");

    //remove obsolete handlers
    auto aimedAPIVersion = this->apiVersion();
    auto handledAPIVersions = handlers.keys().toSet();

    //apply handlers
    bool updateApplied = false;
    for(const auto targetUpdateVersion : handledAPIVersions) {
        
        //if patch is for later versions, skip
        if(aimedAPIVersion < targetUpdateVersion) continue;

        //if target version is not newer than actual DB version, skip
        if(targetUpdateVersion <= databaseToUpdateVersion) continue;

        //update...
        auto msg = QStringLiteral(u"updating from %1 to %2 ...")
                        .arg(databaseToUpdateVersion)
                        .arg(targetUpdateVersion);
        this->log(msg);
                 
        handlers.value(targetUpdateVersion)(databaseToUpdate);
        updateApplied = true;
    }

    //if no update have been applied
    if(!updateApplied) return defaultBehavior("No later handlers found");

    //force version update
    databaseToUpdate.insert(QStringLiteral(u"version"), aimedAPIVersion);
    
    #ifdef NDEBUG
        this->_duplicateDbFile("oldVersion");
    #else
        this->_updateDbFile(databaseToUpdate); //save into file
    #endif

    //end...
    this->log("Update complete !");
    return false;

}

void JSONDatabase::_updateDbFile(const QJsonObject &updatedFullDatabase) {
    if(!this->_destfile) return;
    saveAsFile(updatedFullDatabase, *this->_destfile);
}

QJsonObject JSONDatabase::_emptyDbFile() {
    
    QJsonObject defaultDoc;
    defaultDoc.insert("version", this->apiVersion()); //base mandatory values
    
    //iterate through model
    auto model = this->_getDatabaseModel();
    for(const auto &[key, entityType] : this->_getDatabaseModel().keys()) {
        
        QJsonValue val;
        switch (entityType) {
            case JSONDatabase::EntityType::Object:
                val = QJsonObject();
                break;
            
            case JSONDatabase::EntityType::Array:
                val = QJsonArray();
                break;
        }
        
        defaultDoc.insert(key, val);
        
    }

    return defaultDoc;

}

QJsonDocument JSONDatabase::_readAsDocument() {
    
    if(!this->_destfile) return QJsonDocument();

    this->_destfile->open(QFile::ReadOnly);
        auto readBytes = this->_destfile->readAll();
    this->_destfile->close();

    return QJsonDocument::fromJson(readBytes);
    
}


void JSONDatabase::_duplicateDbFile(QString destSuffix) {
    if(!this->_destfile) return;

    QDir fHandler;
    const auto withSuffixPath = this->_destfile->fileName() + "_" + destSuffix;
    fHandler.remove(withSuffixPath); //remove previous instance of the file if exists
    fHandler.rename(this->_destfile->fileName(), withSuffixPath); //rename current file
}

void JSONDatabase::_removeDatabaseLinkedFiles() {
    //to implement from inheritors
}

const QJsonObject JSONDatabase::_updatedInnerDb(){
    return this->_dbCopy;
}

void JSONDatabase::save() {
    
    //update inner db
    this->_dbCopy = this->_updatedInnerDb();

    if(!this->_destfile) return;
    saveAsFile(this->_dbCopy, *this->_destfile);

    this->log("saved!");

}

void JSONDatabase::saveAsFile(const QJsonObject &db, const QString &filepath) {
    QFile fh(filepath);
    saveAsFile(db, fh);
}

void JSONDatabase::saveAsFile(const QJsonObject &db, QFile &fileHandler) {
    
    QJsonDocument doc(db);

    //define format
    auto format = QJsonDocument::JsonFormat::Compact;
    #ifdef NDEBUG
        format =  QJsonDocument::JsonFormat::Indented;
    #endif
    
    auto bytes = doc.toJson(format);

    fileHandler.open(QFile::WriteOnly);
        fileHandler.write(bytes);
    fileHandler.close();

}

QJsonArray JSONDatabase::diff(QJsonArray &target, QSet<QString> &toRemoveFromTarget) {
    QJsonArray output;
    for(const auto e : target) {
        auto str = e.toString();
        if(!toRemoveFromTarget.contains(str)) {
            output.append(str);
        }
    }
    return output;
}
