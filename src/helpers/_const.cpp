#pragma once

#include <string>
#include <vector>

#include "src/version.h"

#include <QStandardPaths>
#include <QString>
#include <QDir>

#define _WINSOCKAPI_
#include <windows.h>

static const std::string LOCAL_ICON_PNG_PATH = ":/icons/rpgrpz.png";
static const std::string LOG_FILE = "/debug.log";
static const std::string LATEST_LOG_FILE = "/debug.latest.log";
static const std::string UPNP_DEFAULT_TARGET_PORT = "31137";
static const std::string UPNP_REQUEST_DESCRIPTION = "RPGRPZ";

static std::string getAppDataLocation() {
    auto target = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toStdString();
    QDir().mkpath(QString::fromStdString(target));
    return target;
}

static std::string getLogFileLocation() {
    return getAppDataLocation() + LOG_FILE;
}


static std::string getLatestLogFileLocation() {
    return getAppDataLocation() + LATEST_LOG_FILE;
}

static void openFileInOS(std::string cpURL) {
    ShellExecuteA(NULL, "open", "notepad", cpURL.c_str(), NULL, SW_SHOWNORMAL);
};

