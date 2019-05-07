#include "JSONDatabase.h"

JSONDatabase::JSONDatabase() {}

void JSONDatabase::_instanciateDb() {
    
    //read coordinator file as JSON
    auto dbPath = this->dbPath();
    QFile jsonFile(dbPath);
    
    //helper for file creation
    auto writeNewCoord = [&](QFile &file) {
        file.open(QFile::WriteOnly);
        file.write(this->defaultJsonDoc().toStdString().c_str());
        file.close();
    };

    //helper for reading
    auto readNewCoord = [](QFile &file) {
        file.open(QFile::ReadOnly);
        auto coordinator = QJsonDocument::fromJson(file.readAll());
        file.close();
        return coordinator;
    };


    //if file is empty or doesnt exist
    if(!jsonFile.size() || !jsonFile.exists()) {
        writeNewCoord(jsonFile);
    }

    //try to read the file
    auto coordinator = readNewCoord(jsonFile);

    //corrupted file, move it and create a new one
    if(coordinator.isNull()) {
        const auto errorPath = dbPath + "_error";
        QDir().remove(errorPath);
        QDir().rename(dbPath, errorPath);
        writeNewCoord(jsonFile);
        readNewCoord(jsonFile);
    }

    this->_db = new QJsonDocument(coordinator);
}
