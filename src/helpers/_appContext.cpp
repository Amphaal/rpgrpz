// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical or audio resources available within the source code may
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GPL.

#include "_appContext.h"

void AppContext::configureApp(QCoreApplication* app) {
    // context preparation
    app->setApplicationName(QString(APP_NAME));
    app->setOrganizationName(QString(APP_PUBLISHER));

    // define context
    auto args = AppContext::getOptionArgs(app);

    // if custom context is set
    if (args.contains(QStringLiteral(u"randomContext"))) {
        return AppContext::initRandomContext();
    } else if (args.contains(QStringLiteral(u"customContext"))) {
        auto customContext = args.value(QStringLiteral(u"customContext"));
        return AppContext::initCustomContext(customContext);
    }

    // else default init
    AppContext::init();
}

void AppContext::defineMapWidget(QGLWidget* mapGLWidget) {
    _mapGLWidget = mapGLWidget;
}

QGLWidget* AppContext::mapGLWidget() {
    return _mapGLWidget;
}

void AppContext::defineFogOpacity(qreal opacity) {
    _fogOpacity = opacity;
}

qreal AppContext::fogOpacity() {
    return _fogOpacity;
}

qreal AppContext::pointPerCentimeters() {
    return _ppcm;
}
void AppContext::definePPcm(QPaintDevice* device) {
    _ppcm = static_cast<double>(device->logicalDpiX()) / 2.54;
}

void AppContext::installTranslations(QApplication* app) {
    auto translationsPath = app->applicationDirPath() + QDir::separator() + "translations";
    auto locale = QLocale::system();

    // Qt
    if (_qtTranslator.load(locale, "qt", "_", translationsPath)) {
        auto installed = app->installTranslator(&_qtTranslator);
        if (installed) qDebug() << "QT translation installed !";
    }

    // app
    if (_appTranslator.load(locale, "", "", translationsPath)) {
        auto installed = app->installTranslator(&_appTranslator);
        if (installed) qDebug() << "App translation installed !";
    }
}

AppSettings* AppContext::settings() {
    if (!_settings) {
        auto path = getAppDataLocation() + APP_SETTINGS_FILENAME;
        _settings = new AppSettings(path);
    }

    return _settings;
}

const QString AppContext::getWindowTitle() {
    QString stdTitle = APP_FULL_DENOM;

    #ifdef _DEBUG
        stdTitle = "DEBUG - " + stdTitle;
    #endif

    return stdTitle;
}

const QString AppContext::_defaultAppDataLocation() {
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

QHash<QString, QString> AppContext::getOptionArgs(QCoreApplication* source) {
    auto args = source->arguments();
    if (args.count() < 2) return QHash<QString, QString>();
    return _getOptionArgs(args.value(1));
}

QHash<QString, QString> AppContext::_getOptionArgs(const QString &argsAsStr) {
    QHash<QString, QString> out;

    QRegularExpression split("--.+?($|\\s)");

    auto splitMatches = split.globalMatch(argsAsStr);
    while (splitMatches.hasNext()) {
        QRegularExpressionMatch splitMatch = splitMatches.next();  // next

        auto arg = splitMatch.captured();
        auto kvpSplit = arg.split("=", Qt::SkipEmptyParts);

        auto key = kvpSplit.value(0).mid(2).trimmed();
        QString value = kvpSplit.count() > 1 ? kvpSplit.value(1).trimmed() : "";

        out.insert(key, value);
    }

    return out;
}

void AppContext::clearFileSharingFolder() {
    QDir dir(getFileSharingFolderLocation());
    dir.setNameFilters(QStringList() << "*.*");
    dir.setFilter(QDir::Files);

    foreach(QString dirFile, dir.entryList()) {
        dir.remove(dirFile);
    }
}

QHash<QString, QString> AppContext::getOptionArgs(int argc, char** argv) {
    if (argc < 2) return QHash<QString, QString>();
    return _getOptionArgs(QString(argv[1]));
}

void AppContext::initRandomContext() {
    auto templateStr = QStringLiteral(u"%1/r_%2");
    auto randomSF = QString::number(SnowFlake::get()->nextId());

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
    if (!customContext.isEmpty()) {
        _appDataLocation = customContext;
    } else {
        _appDataLocation = _defaultAppDataLocation();
    }

    // update message handler message handler
    qInstallMessageHandler(LogWriter::customMO);

    // log SLL lib loading
    qDebug() << qUtf8Printable(QSslSocket::sslLibraryBuildVersionString());
    qDebug() << qUtf8Printable(QSslSocket::sslLibraryVersionString());

    qDebug() << "Context : using" << _appDataLocation;

    // create default paths
    _makeSureDirPathExists(getAppDataLocation());
    _makeSureDirPathExists(getAssetsFolderLocation());
    _makeSureDirPathExists(getMapsFolderLocation());
    _makeSureDirPathExists(getServerMapAutosaveFolderLocation());
    _makeSureDirPathExists(getFileSharingFolderLocation());

    clearFileSharingFolder();

    // crashpad activated if release app
    // initSentry();
}

// void AppContext::initSentry() {

//     auto options = sentry_options_new();
//     sentry_options_set_dsn(options, SENTRY_ENDPOINT);

//     QString environement = "Production";
//     #ifdef _DEBUG
//         environement = "Debug";
//     #endif
//     sentry_options_set_environment(options, qUtf8Printable(environement));

//     sentry_options_set_release(options, GITHUB_VERSION_NAME);
//     sentry_options_set_debug(options, 1);

//     //integration
//     auto dbStr = AppContext::getAppDataLocation() + "/sentry_db";
//     sentry_options_set_database_path(options, qUtf8Printable(dbStr));

//     sentry_init(options);

// }

void AppContext::_makeSureDirPathExists(const QString &path) {
    struct stat buffer;
    auto pathExists = (stat(qUtf8Printable(path), &buffer) == 0);
    if (!pathExists) QDir().mkpath(path);
}

const QString AppContext::getAppDataLocation() {
    return _appDataLocation;
}

const QString AppContext::getAssetsFileCoordinatorLocation() {
    return getAppDataLocation() + ASSETS_JSON_COORDINATOR_FILENAME;
}

const QString AppContext::getPlaylistFileLocation() {
    return getAppDataLocation() + PLAYLIST_JSON_DATABASE_FILENAME;
}

const QString AppContext::getFileSharingFolderLocation() {
    return getAppDataLocation() + FILE_SHARING_PATH;
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
