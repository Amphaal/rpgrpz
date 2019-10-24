#include "JSONDatabase.h"

JSONDatabase::JSONDatabase() {}

JSONDatabase::JSONDatabase(const QJsonObject &obj) {
    this->_setupFromDbCopy(obj);
}

JSONDatabase::JSONDatabase(const QString &dbFilePath) {
    this->_initDatabaseFromJSONFile(dbFilePath);
}

void JSONDatabase::_initDatabaseFromJSONFile(const QString &dbFilePath) {
    
    //read database file as JSON
    if(this->_destfile) delete this->_destfile;
    this->_destfile = new QFile(dbFilePath);

    //if file is empty or doesnt exist
    if(!this->_destfile->size() || !this->_destfile->exists()) {
        this->_createEmptyDbFile();
    }

    //try to read the file
    auto document = this->_readAsDocument();

    //corrupted file, move it and create a new one
    if(document.isNull()) {
        qDebug() << "JSON Database : Cannot read database, creating a new one...";
        this->_duplicateDbFile("error");
        this->_createEmptyDbFile();
        this->_dbCopy = this->_readAsDocument().object();
        return;
    }

    //compare versions
    auto dbCopy = document.object();
    auto currentVersion = _getDbVersion(dbCopy);
    auto expectedVersion = this->apiVersion();
    if(expectedVersion != currentVersion) {
        
        //duplicate file to migrate
        qDebug() << "JSON Database : Database does not match API version !";
        this->_duplicateDbFile("oldVersion");

        //handle missmatch and update temporary database accordingly
        auto error = this->_handleVersionMissmatch(dbCopy, currentVersion);
        if(error) this->_dbCopy = this->_readAsDocument().object();
        return;
    }

    this->_setupFromDbCopy(dbCopy);

}

void JSONDatabase::_setupFromDbCopy(const QJsonObject &copy) {
    this->_dbCopy = copy;
    this->_setupLocalData();
}

const QString JSONDatabase::dbFilePath() {
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

void JSONDatabase::updateFrom(QJsonObject &base, const QString &entityKey, const QVariantHash &entity) {
    base.insert(entityKey, QJsonObject::fromVariantHash(entity));
}

QHash<JSONDatabase::Version, JSONDatabase::UpdateHandler> JSONDatabase::_getUpdateHandlers() {
    return QHash<JSONDatabase::Version, JSONDatabase::UpdateHandler>();
}

bool JSONDatabase::_handleVersionMissmatch(QJsonObject &databaseToUpdate, JSONDatabase::Version databaseToUpdateVersion) {
    
    auto defaultBehavior = [&](const QString &reason){
        qDebug() << "JSON Database : Database have not been updated :" << reason;
        this->_createEmptyDbFile();
        qDebug() << "JSON Database : Empty database created !";
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
    for(auto targetUpdateVersion : handledAPIVersions) {
        
        //if patch is for later versions, skip
        if(aimedAPIVersion < targetUpdateVersion) continue;

        //if target version is not newer than actual DB version, skip
        if(targetUpdateVersion <= databaseToUpdateVersion) continue;

        //update...
        qDebug() << "JSON Database : updating from" 
                 << QString::number(databaseToUpdateVersion) 
                 << "to" << QString::number(targetUpdateVersion) 
                 << "...";
                 
        handlers.value(targetUpdateVersion)(databaseToUpdate);
        updateApplied = true;
    }

    //if no update have been applied
    if(!updateApplied) return defaultBehavior("No later handlers found");

    //force version update
    databaseToUpdate.insert(QStringLiteral(u"version"), aimedAPIVersion);
    
    //save into file
    this->_updateDbFile(databaseToUpdate);
    qDebug() << "JSON Database : Update complete !";
    return false;

}

void JSONDatabase::_createEmptyDbFile() {

    QJsonObject defaultDoc;
    defaultDoc.insert("version", this->apiVersion()); //base mandatory values
    
    //iterate through model
    auto model = this->_getDatabaseModel();
    for(auto &key : this->_getDatabaseModel().keys()) {
        
        QJsonValue val;
        switch (key.second) {
            case JSONDatabase::EntityType::Object:
                val = QJsonObject();
                break;
            
            case JSONDatabase::EntityType::Array:
                val = QJsonArray();
                break;
        }
        
        defaultDoc.insert(key.first, val);
        
    }

    //update or create file
    this->_updateDbFile(defaultDoc);

}

QJsonDocument JSONDatabase::_readAsDocument() {
    
    if(!this->_destfile) return;

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

void JSONDatabase::_updateDbFile(const QJsonObject &updatedFullDatabase) {
    
    this->_dbCopy = updatedFullDatabase;

    if(!this->_destfile) return;

    QJsonDocument doc(updatedFullDatabase);
    auto format = IS_DEBUG_APP ? QJsonDocument::JsonFormat::Indented : QJsonDocument::JsonFormat::Compact;
    auto bytes = doc.toJson(format);

    this->_destfile->open(QFile::WriteOnly);
        this->_destfile->write(bytes);
    this->_destfile->close();

}

QJsonArray JSONDatabase::diff(QJsonArray &target, QSet<QString> &toRemoveFromTarget) {
    QJsonArray output;
    for(auto e : target) {
        auto str = e.toString();
        if(!toRemoveFromTarget.contains(str)) {
            output.append(str);
        }
    }
    return output;
}
