#pragma once

#include <QTcpSocket>
#include <QString>
#include <QVariant>

class JSONRouter {
    protected:
       virtual void _routeIncomingJSON(JSONSocket * wrapper, QString method, QVariant data) = 0;
};