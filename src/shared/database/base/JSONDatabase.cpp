#include "JSONDatabase.h"

JSONDatabase::JSONDatabase() {}

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
        database = this->_readDbFile();
    }

    //compare versions
    auto dbVersion = this->dbVersion();
    if(this->apiVersion() != dbVersion) {
        
        //duplicate file to migrate
        qDebug() << "JSON Database : Database does not match API version !";
        this->_duplicateDbFile("oldVersion");

        //handle missmatch and update temporary database accordingly
        this->_handleVersionMissmatch(database, dbVersion);
    }

    this->_db = database;
}

QHash<JSONDatabaseVersion, JSONDatabaseUpdateHandler> JSONDatabase::_getUpdateHandlers() {
    return QHash<JSONDatabaseVersion, JSONDatabaseUpdateHandler>();
}

void JSONDatabase::_handleVersionMissmatch(QJsonDocument &databaseToUpdate, int databaseToUpdateVersion) {
    
    auto defaultBehavior = [&](QString reason){
        qDebug() << "JSON Database : Database have not been updated >> " << reason << " Replaced by empty database.";
        this->_createEmptyDbFile();
    };

    //get handlers
    auto handlers = this->_getUpdateHandlers();

    //if no handlers
    if(!handlers.count()) return defaultBehavior("no handlers");

    //remove obsolete handlers
    auto aimedAPIVersion = this->apiVersion();
    auto handledAPIVersions = handlers.keys().toSet();

    //apply handlers
    bool updateApplied = false;
    for(auto targetUpdateVersion : handledAPIVersions) {
        
        //if target version is older than actual version, skip
        if(targetUpdateVersion <= aimedAPIVersion) continue;

        //update...
        qDebug() << "JSON Database : updating from" << QString::number(databaseToUpdateVersion) << "to" << QString::number(targetUpdateVersion) << "...";
        handlers[targetUpdateVersion](databaseToUpdate);
        updateApplied = true;
    }

    //if no update have been applied
    if(!updateApplied) return defaultBehavior("no later handlers");

    //save into file
    this->_updateDbFile(databaseToUpdate.object());
    qDebug() << "JSON Database : Update complete !";

}

void JSONDatabase::_createEmptyDbFile() {
    this->_destfile->open(QFile::WriteOnly);
    this->_destfile->write(this->defaultJsonDoc().toStdString().c_str());
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

void JSONDatabase::_updateDbFile(QJsonObject &newData) {
    this->_destfile->open(QFile::WriteOnly);
    
        this->_db.setObject(newData);
        
        this->_destfile->write(
            this->_db.toJson(
                // QJsonDocument::JsonFormat::Compact
                QJsonDocument::JsonFormat::Indented
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