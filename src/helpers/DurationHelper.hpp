#pragma once

#include <QString>
#include <QtMath>

class DurationHelper {

    public:
        static QString secondsToTrackDuration(int durationInSeconds) {
            QString durationStr;

            int secondsPart = durationInSeconds % 60;
            int minutesPart = (durationInSeconds / 60) % 60;
            int hoursPart = (durationInSeconds / (60 * 60));

            //seconds
            durationStr = ":" + QString::number(secondsPart).rightJustified(2, '0');

            //minutes
            durationStr = QString::number(minutesPart).rightJustified(2, '0') + durationStr;

            if(!hoursPart) return durationStr; //optionnal hours

            //hours
            durationStr = QString::number(hoursPart) + ":" + durationStr;

            return durationStr;
        }

        static QString fromSecondsToTime(int lengthInSeconds) {
            return QTime::fromMSecsSinceStartOfDay(lengthInSeconds * 1000).toString("hh:mm:ss");
        }

};

