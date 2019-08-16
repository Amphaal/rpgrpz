#pragma once

#include <QTcpSocket>
#include <QString>
#include <QVariant>

#include "JSONMethod.h"

class JSONRouter {
    
    protected:
       virtual void _routeIncomingJSON(JSONSocket* target, const JSONMethod &method, const QVariant &data) = 0;
       
};