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
        static const QString _defaultAppDataLocation();
        
        static QHash<QString, QString> _getOptionArgs(const QString &argsAsStr);

        static inline const QString DEBUG_APP_FILE_APPENDICE = IS_DEBUG_APP ? ".debug" : "";
        static inline const QString LOG_FILE = "/rpgrpz" + DEBUG_APP_FILE_APPENDICE + ".log";
        static inline const QString LATEST_LOG_FILE = "/rpgrpz.latest" + DEBUG_APP_FILE_APPENDICE + ".log";
        static inline const QString ASSETS_PATH = "/resources";
        static inline const QString MAPS_PATH = "/maps";
        static inline const QString MAPS_SERVER_AUTOSAVE_PATH = "/serverAutosaves";
        static inline const QString ASSETS_JSON_COORDINATOR_FILENAME = "/resources.json";
        static inline const QString CHARACTER_JSON_DATABASE_FILENAME = "/characters.json";
        static inline const QString APP_SETTINGS_FILENAME = "/settings.ini";
        
        static void _makeSureDirPathExists(const QString &path);

        static inline qreal _ppcm;

    public:    
        static AppSettings* settings();

        static inline const QString UPNP_DEFAULT_TARGET_PORT = "31137";
        static inline const QString UPNP_REQUEST_DESCRIPTION = APP_NAME;
        static inline const QString RPZ_MAP_FILE_EXT = ".mrpz";
        static inline constexpr int TOP_Z_INDEX = 100000;

        static inline const QString REGEX_YOUTUBE_URL = "(?:youtube\\.com|youtu.be).*?(?:v=|embed\\/)(?<videoId>[\\w\\-]+)";
        static inline const QString REGEX_URL = "(https?:\\/\\/(?:www\\.|(?!www))[a-zA-Z0-9][a-zA-Z0-9-]+[a-zA-Z0-9]\\.[^\\s]{2,}|www\\.[a-zA-Z0-9][a-zA-Z0-9-]+[a-zA-Z0-9]\\.[^\\s]{2,}|https?:\\/\\/(?:www\\.|(?!www))[a-zA-Z0-9]+\\.[^\\s]{2,}|www\\.[a-zA-Z0-9]+\\.[^\\s]{2,})";

        static QHash<QString, QString> getOptionArgs(QCoreApplication &source);
        static QHash<QString, QString> getOptionArgs(int argc, char** argv);
        static void configureApp(QCoreApplication &app);

        static void definePPcm(QPaintDevice* device);
        static qreal pointPerCentimeters();

        void static initRandomContext();
        void static initCustomContext(const QString &customContextSuffix);
        void static init(const QString &customContext = QString());

        static const QString getAppDataLocation();
        static const QString getWindowTitle();
        static const QString getAssetsFileCoordinatorLocation();
        static const QString getCharacterDatabaseLocation();
        static const QString getAssetsFolderLocation();
        static const QString getMapsFolderLocation();
        static const QString getDefaultMapFilePath();
        static const QString getLogFileLocation();
        static const QString getLatestLogFileLocation();
        static const QString getServerMapAutosaveFolderLocation();

        static void openFileInOS(const QString &cpURL);
        static void openFolderInOS(const QString &cpURL);

        static void installTranslations(QApplication &app);
};
