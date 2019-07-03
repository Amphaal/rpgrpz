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
    QString customContext = NULL;
    
    //if custom context is set
    if(args.count() > 1) {

        customContext = args[1];
        if(customContext == "random") {
            return AppContext::initRandomContext();
        } else {
            return AppContext::initCustomContext(customContext);
        }
    }

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

QStringList AppContext::getOptionArgs(QCoreApplication &source) {
    auto args = source.arguments();
    if(args.count() < 2) return QStringList();

    return args[1].split(" ", QString::SplitBehavior::SkipEmptyParts);
}

QStringList AppContext::getOptionArgs(int argc, char** argv) {
    if(argc < 2) return QStringList();

    return QString(argv[1]).split(" ", QString::SplitBehavior::SkipEmptyParts);
}

void AppContext::initRandomContext() {
    QTemporaryDir randomDir(_defaultAppDataLocation() + "/r");
    init(randomDir.path());
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
    
} 


QString AppContext::makeSureDirPathExists(const QString &path) {
    QDir().mkpath(path);
    return path;
}

QString AppContext::getAppDataLocation() {
    return makeSureDirPathExists(_appDataLocation);
}

QString AppContext::getWindowTitle() {
    QString stdTitle = APP_FULL_DENOM;
    if(IS_DEBUG_APP) stdTitle = "DEBUG - " + stdTitle;
    return stdTitle;
}

QString AppContext::getAssetsFileCoordinatorLocation() {
    return getAppDataLocation() + ASSETS_JSON_COORDINATOR_FILENAME;
}

QString AppContext::getAssetsFolderLocation() {
    auto dest = getAppDataLocation() + ASSETS_PATH;
    return makeSureDirPathExists(dest);
}

QString AppContext::getMapsFolderLocation() {
    auto dest = getAppDataLocation() + MAPS_PATH;
    return makeSureDirPathExists(dest);
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
     QProcess::startDetached("explorer.exe", {"/select,", QDir::toNativeSeparators(cpURL)});
};

