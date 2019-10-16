#pragma once

#include <sys/stat.h>

#include "src/version.h"
#include <iostream>

#include <QtCore/QCoreApplication>

#include <QtCore/QSettings>

#include <QtCore/QStandardPaths>
#include <QtCore/QTemporaryDir>
#include <QtCore/QString>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QDateTime>

#include <QtGui/QDesktopServices>
#include <QtCore/QProcess>
#include <QtCore/QUrl>
#include <QTranslator>
#include <QMessageBox>
#include <QApplication>
#include <QLibraryInfo>

#include <QHash>

#include <QRegularExpression>

#include "_appSettings.hpp"

#include "RPZUserRoles.h"
#include "src/_libs/snowflake/snowflake.h"

class AppContext {
    
    private:
        static inline AppSettings* _settings = nullptr;

        static inline QString _appDataLocation;
        static QString _defaultAppDataLocation();
        
        static QHash<QString, QString> _getOptionArgs(const QString &argsAsStr);

        static inline const QString DEBUG_APP_FILE_APPENDICE = IS_DEBUG_APP ? ".debug" : "";
        static inline const QString LOG_FILE = "/rpgrpz" + DEBUG_APP_FILE_APPENDICE + ".log";
        static inline const QString LATEST_LOG_FILE = "/rpgrpz.latest" + DEBUG_APP_FILE_APPENDICE + ".log";
        static inline const QString ASSETS_PATH = "/resources";
        static inline const QString MAPS_PATH = "/maps";
        static inline const QString ASSETS_JSON_COORDINATOR_FILENAME = "/resources.json";
        static inline const QString CHARACTER_JSON_DATABASE_FILENAME = "/characters.json";
        static inline const QString APP_SETTINGS_FILENAME = "/settings.ini";
        
        static void _makeSureDirPathExists(const QString &path);

    public:    
        static AppSettings* settings();

        static inline const QString UPNP_DEFAULT_TARGET_PORT = "31137";
        static inline const QString UPNP_REQUEST_DESCRIPTION = APP_NAME;
        static inline const QString RPZ_MAP_FILE_EXT = ".mrpz";

        static inline const QString REGEX_YOUTUBE_URL = "(?:youtube\\.com|youtu.be).*?(?:v=|embed\\/)(?<videoId>[\\w\\-]+)";

        static QHash<QString, QString> getOptionArgs(QCoreApplication &source);
        static QHash<QString, QString> getOptionArgs(int argc, char** argv);
        static void configureApp(QCoreApplication &app);

        static QSizeF standardTileSize(QPaintDevice* device);

        void static initRandomContext();
        void static initCustomContext(const QString &customContextSuffix);
        void static init(const QString &customContext = QString());

        static QString getAppDataLocation();
        static QString getWindowTitle();
        static QString getAssetsFileCoordinatorLocation();
        static QString getCharacterDatabaseLocation();
        static QString getAssetsFolderLocation();
        static QString getMapsFolderLocation();
        static QString getDefaultMapFile();
        static QString getLogFileLocation();
        static QString getLatestLogFileLocation();

        static void openFileInOS(const QString &cpURL);
        static void openFolderInOS(const QString &cpURL);

        static void installTranslations(QApplication &app);
};
