#include "JSONDatabase.h"

JSONDatabase::JSONDatabase() {}

const int JSONDatabase::dbVersion() {
    return getDbVersion(this->_db);
}

JSONDatabaseVersion JSONDatabase::getDbVersion(QJsonDocument &db) {
    auto version = db["version"].toInt();
    return version;
}

void JSONDatabase::_instanciateDb() {
    
    //read database file as JSON
    auto dbPath = this->dbPath();
    this->_destfile = new QFile(dbPath);

    //if file is empty or doesnt exist
    if(!this->_destfile->size() || !this->_destfile->exists()) {
        this->_createEmptyDbFile();
    }

    //try to read the file
    auto database = this->_readDbFile();

    //corrupted file, move it and create a new one
    if(database.isNull()) {
        qDebug() << "JSON Database : Cannot read database, creating a new one...";
        this->_duplicateDbFile("error");
        this->_createEmptyDbFile();
        this->_db = this->_readDbFile();
        return;
    }

    //compare versions
    auto currentVersion = getDbVersion(database);
    auto expectedVersion = this->apiVersion();
    if(expectedVersion != currentVersion) {
        
        //duplicate file to migrate
        qDebug() << "JSON Database : Database does not match API version !";
        this->_duplicateDbFile("oldVersion");

        //handle missmatch and update temporary database accordingly
        auto error = this->_handleVersionMissmatch(database, currentVersion);
        if(error) this->_db = this->_readDbFile();
        return;
    }

    //bind
    this->_db = database;
}

QHash<JSONDatabaseVersion, JSONDatabaseUpdateHandler> JSONDatabase::_getUpdateHandlers() {
    return QHash<JSONDatabaseVersion, JSONDatabaseUpdateHandler>();
}

bool JSONDatabase::_handleVersionMissmatch(QJsonDocument &databaseToUpdate, int databaseToUpdateVersion) {
    
    auto defaultBehavior = [&](QString reason){
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
                 
        handlers[targetUpdateVersion](databaseToUpdate);
        updateApplied = true;
    }

    //if no update have been applied
    if(!updateApplied) return defaultBehavior("No later handlers found");

    //force version update
    auto out = databaseToUpdate.object();
    out["version"] = aimedAPIVersion;
    
    //save into file
    this->_updateDbFile(out);
    qDebug() << "JSON Database : Update complete !";
    return false;

}

void JSONDatabase::_createEmptyDbFile() {
    this->_destfile->open(QFile::WriteOnly);
    this->_destfile->write(qUtf8Printable(this->defaultJsonDoc()));
    this->_destfile->close();
}

QJsonDocument JSONDatabase::_readDbFile() {
    this->_destfile->open(QFile::ReadOnly);
    auto readBytes = this->_destfile->readAll();
    auto database = QJsonDocument::fromJson(readBytes);
    this->_destfile->close();
    return database;
}


void JSONDatabase::_duplicateDbFile(QString destSuffix) {
    QDir fHandler;
    const auto withSuffixPath = this->dbPath() + "_" + destSuffix;
    fHandler.remove(withSuffixPath); //remove previous instance of the file if exists
    fHandler.rename(this->dbPath(), withSuffixPath); //rename current file
}

void JSONDatabase::_checkFileExistance() {
    if(!this->_destfile->exists()) this->_instanciateDb();
}

void JSONDatabase::_removeDatabaseLinkedFiles() {
    //to implement from inheritors
}

void JSONDatabase::_updateDbFile(const QJsonObject &newData) {
    this->_destfile->open(QFile::WriteOnly);
    
        this->_db.setObject(newData);
        
        this->_destfile->write(
            this->_db.toJson(
                QJsonDocument::JsonFormat::Compact
                //QJsonDocument::JsonFormat::Indented
            )
        );

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