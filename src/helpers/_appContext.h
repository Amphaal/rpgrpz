#pragma once

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

#include <QHash>

#include <QRegularExpression>

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
        
        static QHash<QString, QString> _getOptionArgs(const QString &argsAsStr);

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

        static inline const QString REGEX_YOUTUBE_URL = "^((?:https?:)?\\/\\/)?((?:www|m)\\.)?((?:youtube\\.com|youtu.be))(\\/(?:[\\w\\-]+\?v=|embed\\/|v\\/)?)([\\w\\-]+)(\\S+)?$";

        static QHash<QString, QString> getOptionArgs(QCoreApplication &source);
        static QHash<QString, QString> getOptionArgs(int argc, char** argv);
        static void configureApp(QCoreApplication &app);

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
