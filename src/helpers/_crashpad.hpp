#pragma once

#include <map>
#include <string>
#include <vector>

#include "client/crashpad_client.h"
#include "client/settings.h"
#include "client/crash_report_database.h"

#include "_appContext.h"

using namespace crashpad;

bool startCrashpad() {

  // Cache directory that will store crashpad information and minidumps
  auto dbStr = AppContext::getAppDataLocation() + "/crashpad_db";
  base::FilePath database(dbStr.toStdWString());

  // Path to the out-of-process handler executable
  auto handlerStr = QString(CRASHPAD_HANDLER_NAME);
  base::FilePath handler(handlerStr.toStdWString());

  // URL used to submit minidumps to
  std::string url(SENTRY_ENDPOINT);

  // Optional annotations passed via --annotations to the handler
  std::map<std::string, std::string> annotations;

  // Optional arguments to pass to the handler
  std::vector<std::string> arguments;
  arguments.push_back("--no-rate-limit");

  CrashpadClient client;
  bool success = client.StartHandler(
    handler,
    database,
    database,
    url,
    annotations,
    arguments,
    /* restartable */ true,
    /* asynchronous_start */ false
  );


  //
  std::unique_ptr<CrashReportDatabase> db = CrashReportDatabase::Initialize(database);
  if (db != nullptr && db->GetSettings() != nullptr) {
    db->GetSettings()->SetUploadsEnabled(true);
  }

  return success;

}