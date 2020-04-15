// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical or audio resources available within the source code may 
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GNU General Public License.

#pragma once

#include <QString>
#include <QTime>

class StringHelper {
    public:

        static QString fromMeters(double meters) {

            if(meters < 0.1) {
                auto asMm = meters * 1000;
                return QStringLiteral(u"%1 mm").arg(asMm, 4, 0, 1, 0);
            }

            else if(meters < 1) {
                auto asCm = meters * 100;
                return QStringLiteral(u"%1 cm").arg(asCm, 4, 0, 1, 0);
            } 

            else if (meters > 1000){
                auto asKm = meters / 1000;
                return QStringLiteral(u"%1 km").arg(asKm, 4, 0, 1, 0);
            }

            else {
                return QStringLiteral(u"%1 m").arg(meters, 4, 0, 1, 0);
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
            return QTime::fromMSecsSinceStartOfDay(lengthInSeconds * 1000).toString(QStringLiteral(u"hh:mm:ss"));
        }

        static QString toSuperScript(uint num) {
            QString out;
            for(const auto &character : QString::number(num)) {
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