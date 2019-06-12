#pragma once

#include "src/version.h"

#include <QCoreApplication>

#include <QSettings>

#include <QStandardPaths>
#include <QTemporaryDir>
#include <QString>
#include <QDebug>
#include <QDir>
#include <QDateTime>

#define _WINSOCKAPI_
#include <windows.h>

class AppSettings : public QSettings {
    public:
        AppSettings(const QString &path);

        int audioVolume();
        void setAudioVolume(int volume = 100);

        int defaultLayer();
        void setDefaultLayer(int layer = 0);
};

class AppContext {
    
    private:
        static inline AppSettings* _settings = nullptr;

        static inline QString _appDataLocation;
        static QString _defaultAppDataLocation();

        static inline const QString DEBUG_APP_FILE_APPENDICE = IS_DEBUG_APP ? ".debug" : "";
        static inline const QString LOG_FILE = "/rpgrpz" + DEBUG_APP_FILE_APPENDICE + ".log";
        static inline const QString LATEST_LOG_FILE = "/rpgrpz.latest" + DEBUG_APP_FILE_APPENDICE + ".log";
        static inline const QString ASSETS_PATH = "/resources";
        static inline const QString MAPS_PATH = "/maps";
        static inline const QString ASSETS_JSON_COORDINATOR_FILENAME = "/resources.json";
        static inline const QString APP_SETTINGS_FILENAME = "/settings.ini";
        
    public:    
        static AppSettings* settings();

        static inline const QString UPNP_DEFAULT_TARGET_PORT = "31137";
        static inline const QString UPNP_REQUEST_DESCRIPTION = "RPGRPZ";
        static inline const QString RPZ_MAP_FILE_EXT = ".mrpz";

        static QStringList getOptionArgs(QCoreApplication &source);
        static QStringList getOptionArgs(int argc, char** argv);

        void static initRandomContext();
        void static initCustomContext(const QString &customContextSuffix);
        void static init(const QString &customContext = QString());

        static QString makeSureDirPathExists(const QString &path);
        static QString getAppDataLocation();
        static QString getWindowTitle();
        static QString getAssetsFileCoordinatorLocation();
        static QString getAssetsFolderLocation();
        static QString getMapsFolderLocation();
        static QString getDefaultMapFile();
        static QString getLogFileLocation();
        static QString getLatestLogFileLocation();

        static void openFileInOS(const QString &cpURL);
        static void openFolderInOS(const QString &cpURL);
};