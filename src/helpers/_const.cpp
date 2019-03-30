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
static const std::string LOCAL_ICON_PNG_PATH = ":/icons/rpgrpz.png";
static const std::string LOG_FILE = "/rpgrpz" + DEBUG_APP_FILE_APPENDICE + ".log";
static const std::string LATEST_LOG_FILE = "/rpgrpz.latest" + DEBUG_APP_FILE_APPENDICE + ".log";
static const std::string UPNP_DEFAULT_TARGET_PORT = "31137";
static const std::string UPNP_REQUEST_DESCRIPTION = "RPGRPZ";

static std::string getAppDataLocation() {
    auto target = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toStdString();
    auto qt = QString::fromStdString(target);
    auto created = QDir().mkpath(qt);
    return target;
}

static QString formatChatMessage(QString displayName, QString message) {
    auto ts = QString("[" + QDateTime::currentDateTime().toString("dd.MM.yyyy-hh:mm:ss") + "] ");
    auto name = displayName + " a dit : ";
    auto fullMsg = ts + name + "“" + message + "”";
    return fullMsg;
}

static std::string getLogFileLocation() {
    return getAppDataLocation() + LOG_FILE;
}

static std::string getLatestLogFileLocation() {
    return getAppDataLocation() + LATEST_LOG_FILE;
}

static void openFileInOS(std::string cpURL) {
    ShellExecute(NULL, "open", cpURL.c_str(), NULL,  NULL, SW_SHOWNORMAL);
};

static void openFolderInOS(std::string cpURL) {
    ShellExecute(NULL, "open",  cpURL.c_str(), NULL, NULL, SW_SHOWNORMAL);
};