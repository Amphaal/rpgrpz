#include "JSONDatabase.h"

JSONDatabase::JSONDatabase() {}

void JSONDatabase::_instanciateDb() {
    
    //read coordinator file as JSON
    auto dbPath = this->dbPath();
    this->_destfile = new QFile(dbPath);
    
    //helper for file creation
    auto writeNewCoord = [&](QFile* file) {
        file->open(QFile::WriteOnly);
        file->write(this->defaultJsonDoc().toStdString().c_str());
        file->close();
    };

    //helper for reading
    auto readNewCoord = [](QFile* file) {
        file->open(QFile::ReadOnly);
        auto coordinator = QJsonDocument::fromJson(file->readAll());
        file->close();
        return coordinator;
    };


    //if file is empty or doesnt exist
    if(!this->_destfile->size() || !this->_destfile->exists()) {
        writeNewCoord(this->_destfile);
    }

    //try to read the file
    auto coordinator = readNewCoord(this->_destfile);

    //corrupted file, move it and create a new one
    if(coordinator.isNull()) {
        const auto errorPath = dbPath + "_error";
        QDir().remove(errorPath);
        QDir().rename(dbPath, errorPath);
        writeNewCoord(this->_destfile);
        coordinator = readNewCoord(this->_destfile);
    }

    this->_db = coordinator;

    //compare versions, if missmatch, delete db and recreate
    if(this->apiVersion() != this->dbVersion()) {
        qDebug() << "Assets : Database removed because of a new API version !";
        this->_removeDatabase();
        writeNewCoord(this->_destfile);
        this->_db = readNewCoord(this->_destfile);
    }
}

void JSONDatabase::_checkFileExistance() {
    if(!this->_destfile->exists()) this->_instanciateDb();
}

void JSONDatabase::_removeDatabase() {
    QDir().remove(this->dbPath());
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
    auto output = QJsonArray();
    for(auto &e : target) {
        auto str = e.toString();
        if(!toRemoveFromTarget.contains(str)) {
            output.append(str);
        }
    }
    return output;
}