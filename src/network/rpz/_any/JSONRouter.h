#pragma once

#include <QTcpSocket>
#include <QString>
#include <QVariant>

#include "JSONMethod.h"

class JSONRouter {
    protected:
       virtual void _routeIncomingJSON(JSONSocket* target, JSONMethod method, QVariant data) = 0;
};