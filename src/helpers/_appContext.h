#pragma once

#include <sys/stat.h>

#include "src/version.h"

#include <QDir>
#include <QDesktopServices>
#include <QProcess>
#include <QTranslator>
#include <QLibraryInfo>
#include <QApplication>
#include <QPaintDevice>
#include <QRegularExpression>
#include <QUrl>

#include "src/helpers/_appSettings.hpp"
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
        static inline const double DEFAULT_TILE_TO_METER_RATIO = 1.5;

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
        static QString getDefaultMapFilePath();
        static QString getLogFileLocation();
        static QString getLatestLogFileLocation();

        static void openFileInOS(const QString &cpURL);
        static void openFolderInOS(const QString &cpURL);

        static void installTranslations(QApplication &app);
};
