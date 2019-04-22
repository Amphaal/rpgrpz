#pragma once

#include <string>
#include <vector>

#include "src/version.h"

#include <QStandardPaths>
#include <QString>
#include <QDebug>
#include <QDir>
#include <QDateTime>

#define _WINSOCKAPI_
#include <windows.h>

static const std::string DEBUG_APP_FILE_APPENDICE = IS_DEBUG_APP ? ".debug" : "";
static const std::string LOG_FILE = "/rpgrpz" + DEBUG_APP_FILE_APPENDICE + ".log";
static const std::string LATEST_LOG_FILE = "/rpgrpz.latest" + DEBUG_APP_FILE_APPENDICE + ".log";
static const std::string ASSETS_PATH = "/resources";
static const std::string ASSETS_JSON_COORDINATOR_FILENAME = "/resources.json";
static const std::string UPNP_DEFAULT_TARGET_PORT = "31137";
static const std::string UPNP_REQUEST_DESCRIPTION = "RPGRPZ";

static std::string getAppDataLocation() {
    auto target = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toStdString();
    auto qt = QString::fromStdString(target);
    QDir().mkpath(qt);
    return target;
}

static std::string getAssetsFileCoordinatorLocation() {
    return getAppDataLocation() + ASSETS_JSON_COORDINATOR_FILENAME;
}

static std::string getAssetsFolderLocation() {
    return getAppDataLocation() + ASSETS_PATH;
}

static std::string getLogFileLocation() {
    return getAppDataLocation() + LOG_FILE;
}

static std::string getLatestLogFileLocation() {
    return getAppDataLocation() + LATEST_LOG_FILE;
}

static void openFileInOS(const std::string &cpURL) {
    ShellExecute(NULL, "open", cpURL.c_str(), NULL,  NULL, SW_SHOWNORMAL);
};

static void openFolderInOS(const std::string &cpURL) {
    ShellExecute(NULL, "open",  cpURL.c_str(), NULL, NULL, SW_SHOWNORMAL);
};