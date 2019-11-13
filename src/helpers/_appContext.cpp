#include "_appContext.h"

void AppContext::configureApp(QCoreApplication &app) {
    
    //context preparation
    app.setApplicationName(QString(APP_NAME));
    app.setOrganizationName(QString(APP_PUBLISHER));
    
    //define context
    auto args = AppContext::getOptionArgs(app);

    //if custom context is set
    if(args.contains(QStringLiteral(u"randomContext"))) {
        return AppContext::initRandomContext();
    } 
    
    else if(args.contains(QStringLiteral(u"customContext"))) {
        auto customContext = args.value(QStringLiteral(u"customContext"));
        return AppContext::initCustomContext(customContext);
    }

    //else default init
    AppContext::init();
}

void AppContext::defineMapWidget(QGLWidget* mapGLWidget) {
    _mapGLWidget = mapGLWidget;
}

QGLWidget* AppContext::mapGLWidget() {
    return _mapGLWidget;
}

qreal AppContext::pointPerCentimeters() {
    return _ppcm;
}

void AppContext::definePPcm(QPaintDevice* device) {
    _ppcm = (double)device->logicalDpiX() / 2.54;
}

void AppContext::installTranslations(QApplication &app) {
    
    QString translationsPath(QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    QLocale locale = QLocale::system();
    
    //Qt
    auto qtTranslator = new QTranslator;
    if (qtTranslator->load(locale, "qt", "_", translationsPath)) {
        auto installed = app.installTranslator(qtTranslator);
    }

    //app
    auto appTranslator = new QTranslator;
    if (appTranslator->load(locale, "", "", translationsPath)) {
        auto installed = app.installTranslator(appTranslator);
    }

}

AppSettings* AppContext::settings() {

    if(!_settings) {
        auto path = getAppDataLocation() + APP_SETTINGS_FILENAME;
        _settings = new AppSettings(path);
    }
    
    return _settings;
}

const QString AppContext::getWindowTitle() {
    QString stdTitle = APP_FULL_DENOM;
    if(IS_DEBUG_APP) stdTitle = "DEBUG - " + stdTitle;
    return stdTitle;
}

const QString AppContext::_defaultAppDataLocation() {
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
} 

QHash<QString, QString> AppContext::getOptionArgs(QCoreApplication &source) {
    auto args = source.arguments();
    if(args.count() < 2) return QHash<QString, QString>();
    return _getOptionArgs(args.value(1));
}

QHash<QString, QString> AppContext::_getOptionArgs(const QString &argsAsStr) {
    QHash<QString, QString> out;

    QRegularExpression split("--.+?($|\\s)");

    auto splitMatches = split.globalMatch(argsAsStr);
    while (splitMatches.hasNext()) {
        QRegularExpressionMatch splitMatch = splitMatches.next(); //next

        auto arg = splitMatch.captured();
        auto kvpSplit = arg.split("=", QString::SkipEmptyParts);

        auto key = kvpSplit.value(0).mid(2).trimmed();
        QString value = kvpSplit.count() > 1 ? kvpSplit.value(1).trimmed() : "";

        out.insert(key, value);
    }
    
    return out;
}

QHash<QString, QString> AppContext::getOptionArgs(int argc, char** argv) {
    
    if(argc < 2) return QHash<QString, QString>();

    return _getOptionArgs(
        QString(argv[1])
    );

}

void AppContext::initRandomContext() {
    
    auto templateStr = QStringLiteral(u"%1/r_%2");
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
    _makeSureDirPathExists(getServerMapAutosaveFolderLocation());

} 


void AppContext::_makeSureDirPathExists(const QString &path) {
    struct stat buffer;
    auto pathExists = (stat(qUtf8Printable(path), &buffer) == 0);
    if(!pathExists) QDir().mkpath(path);
}

const QString AppContext::getAppDataLocation() {
    return _appDataLocation;
}

const QString AppContext::getAssetsFileCoordinatorLocation() {
    return getAppDataLocation() + ASSETS_JSON_COORDINATOR_FILENAME;
}

const QString AppContext::getCharacterDatabaseLocation() {
    return getAppDataLocation() + CHARACTER_JSON_DATABASE_FILENAME;
}

const QString AppContext::getAssetsFolderLocation() {
    auto dest = getAppDataLocation() + ASSETS_PATH;
    return dest;
}

const QString AppContext::getMapsFolderLocation() {
    auto dest = getAppDataLocation() + MAPS_PATH;
    return dest;
}

const QString AppContext::getServerMapAutosaveFolderLocation() {
    auto dest = getAppDataLocation() + MAPS_SERVER_AUTOSAVE_PATH;
    return dest;
}

const QString AppContext::getDefaultMapFilePath() {
    return getMapsFolderLocation() + "/default" + RPZ_MAP_FILE_EXT;
}

const QString AppContext::getLogFileLocation() {
    return getAppDataLocation() + LOG_FILE;
}

const QString AppContext::getLatestLogFileLocation() {
    return getAppDataLocation() + LATEST_LOG_FILE;
}

void AppContext::openFileInOS(const QString &cpURL) {
    QUrl url("file:///" + cpURL, QUrl::TolerantMode);
    QDesktopServices::openUrl(url);
}

void AppContext::openFolderInOS(const QString &cpURL) {
    auto toOpen = QDir::toNativeSeparators(cpURL);
    QProcess::startDetached("explorer.exe", {toOpen});
}