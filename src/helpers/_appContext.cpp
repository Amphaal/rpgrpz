#include "_appContext.h"

AppSettings::AppSettings(const QString &path) : QSettings(path, QSettings::IniFormat) {}

int AppSettings::audioVolume() {
    return this->value("volume", 50).toInt();
}

void AppSettings::setAudioVolume(int volume) {
    this->setValue("volume", volume);
}

int AppSettings::defaultLayer() {
    return this->value("defaultLayer", 0).toInt();
}
void AppSettings::setDefaultLayer(int layer) {
    this->setValue("defaultLayer", layer);
}

///
///
///

void AppContext::configureApp(QCoreApplication &app) {
    
    //context preparation
    app.setApplicationName(QString(APP_NAME));
    app.setOrganizationName(QString(APP_PUBLISHER));
    
    //define context
    auto args = AppContext::getOptionArgs(app);

    //if custom context is set
    if(args.contains("randomContext")) {
        return AppContext::initRandomContext();
    } 
    
    else if(args.contains("customContext")) {
        auto customContext = args["customContext"];
        return AppContext::initCustomContext(customContext);
    }
    
    //else default init
    AppContext::init();
}


AppSettings* AppContext::settings() {

    if(!_settings) {
        auto path = getAppDataLocation() + APP_SETTINGS_FILENAME;
        _settings = new AppSettings(path);
    }
    
    return _settings;
}

QString AppContext::_defaultAppDataLocation() {
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
} 

QHash<QString, QString> AppContext::getOptionArgs(QCoreApplication &source) {
    auto args = source.arguments();
    if(args.count() < 2) return QHash<QString, QString>();
    return _getOptionArgs(args[1]);
}

QHash<QString, QString> AppContext::_getOptionArgs(const QString &argsAsStr) {
    QHash<QString, QString> out;

    QRegularExpression split("--.+?($|\\s)");

    auto splitMatches = split.globalMatch(argsAsStr);
    while (splitMatches.hasNext()) {
        QRegularExpressionMatch splitMatch = splitMatches.next(); //next

        auto arg = splitMatch.captured();
        auto kvpSplit = arg.split("=", QString::SkipEmptyParts);

        auto key = kvpSplit[0].mid(2).trimmed();
        QString value = kvpSplit.count() > 1 ? kvpSplit[1].trimmed() : "";

        out.insert(key, value);
    }
    
    return out;
}

QHash<QString, QString> AppContext::getOptionArgs(int argc, char** argv) {
    if(argc < 2) return QHash<QString, QString>();

    return _getOptionArgs(QString(argv[1]));
}

void AppContext::initRandomContext() {
    
    QString templateStr("%1/r_%2");
    auto randomSF = QString::number(
        SnowFlake::get()->nextId()
    );

    auto randomPath = templateStr
                        .arg(_defaultAppDataLocation())
                        .arg(randomSF);
                        
    init(randomPath);
}

void AppContext::initCustomContext(const QString &customContextSuffix) {
    auto fullContextPath = _defaultAppDataLocation() + "/" + customContextSuffix;
    init(fullContextPath);
}

void AppContext::init(const QString &customContext) {
    
    if(!customContext.isEmpty()) {
        _appDataLocation = customContext;
    } else {
        _appDataLocation = _defaultAppDataLocation();
    }
    
    qDebug() << "Context : using" << _appDataLocation;

    //create default paths
    _makeSureDirPathExists(getAppDataLocation());
    _makeSureDirPathExists(getAssetsFolderLocation());
    _makeSureDirPathExists(getMapsFolderLocation());

} 


void AppContext::_makeSureDirPathExists(const QString &path) {
    struct stat buffer;
    auto pathExists = (stat(path.toStdString().c_str(), &buffer) == 0);
    if(!pathExists) QDir().mkpath(path);
}

QString AppContext::getAppDataLocation() {
    return _appDataLocation;
}

QString AppContext::getWindowTitle() {
    QString stdTitle = APP_FULL_DENOM;
    if(IS_DEBUG_APP) stdTitle = "DEBUG - " + stdTitle;
    return stdTitle;
}

QString AppContext::getAssetsFileCoordinatorLocation() {
    return getAppDataLocation() + ASSETS_JSON_COORDINATOR_FILENAME;
}

QString AppContext::getCharacterDatabaseLocation() {
    return getAppDataLocation() + CHARACTER_JSON_DATABASE_FILENAME;
}

QString AppContext::getAssetsFolderLocation() {
    auto dest = getAppDataLocation() + ASSETS_PATH;
    return dest;
}

QString AppContext::getMapsFolderLocation() {
    auto dest = getAppDataLocation() + MAPS_PATH;
    return dest;
}

QString AppContext::getDefaultMapFile() {
    return getMapsFolderLocation() + "/default" + RPZ_MAP_FILE_EXT;
}

QString AppContext::getLogFileLocation() {
    return getAppDataLocation() + LOG_FILE;
}

QString AppContext::getLatestLogFileLocation() {
    return getAppDataLocation() + LATEST_LOG_FILE;
}

void AppContext::openFileInOS(const QString &cpURL) {
    QUrl url("file:///" + cpURL, QUrl::TolerantMode);
    QDesktopServices::openUrl(url);
};

void AppContext::openFolderInOS(const QString &cpURL) {
    auto toOpen = QDir::toNativeSeparators(cpURL);
    QProcess::startDetached("explorer.exe", {toOpen});
};