#pragma once

#include <QString>

class StringHelper {
    public:
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