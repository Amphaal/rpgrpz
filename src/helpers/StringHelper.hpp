#pragma once

#include <QString>

class StringHelper {
    public:

        static QString fromcentimeters(qulonglong cm) {

            auto numberLength = QString::number(cm).count();
            if(numberLength < 3) {
                return QString("%1 cm").arg(cm);
            } 
            
            else {
                return QString("%1 m").arg(cm, 0, 0, 3, 0);
            }


        }

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

        static QString toSuperScript(uint num) {
            QString out;
            for(auto &character : QString::number(num)) {
                switch(character.digitValue()) {
                    case 0:
                        out += "⁰";
                        break;
                    case 1:
                        out += "¹";
                        break;
                    case 2:
                        out += "²";
                        break;
                    case 3:
                        out += "³";
                        break;
                    case 4:
                        out += "⁴";
                        break;
                    case 5:
                        out += "⁵";
                        break;
                    case 6:
                        out += "⁶";
                        break;
                    case 7:
                        out += "⁷";
                        break;
                    case 8:
                        out += "⁸";
                        break;
                    case 9:
                        out += "⁹";
                        break;
                }
            }
            return out;
        }
};