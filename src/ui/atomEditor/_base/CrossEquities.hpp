#pragma once

#include <QVector>

class CrossEquities {
    public:
        struct CrossEquity {
            double atomValue;
            int sliderValue;
        };

        CrossEquities(const QVector<CrossEquity> &equities) {
            
            auto equitiesCount = equities.count();
            Q_ASSERT(equitiesCount > 1);

            for(auto i = 1; i < equitiesCount; i++) {
                
                auto sub = i - 1;
                auto bEquity = equities.at(sub);
                auto eEquity = equities.at(i);

                QPointF begin(bEquity.atomValue, bEquity.sliderValue);
                QPointF end(eEquity.atomValue, eEquity.sliderValue);

                this->_lines += { begin, end };

            }

            this->_defineMinMaxes();

        }

        double toAtomValue(int sliderVal) const {
            auto func = this->_functionToUseFromSliderValue(sliderVal);
            auto min = func.y1();
            auto max = func.y2();
            auto prc = (sliderVal - min) / (max - min);
            auto out = func.pointAt(prc).x();
            return out;
        }

        int toSliderValue(double atomVal) const {
            auto func = this->_functionToUseFromAtomValue(atomVal);
            auto min = func.x1();
            auto max = func.x2();
            auto prc = (atomVal - min) / (max - min);
            auto out = qRound(func.pointAt(prc).y());
            return out;
        }

        int maxSlider() const { return this->_maxSlider; }
        int minSlider() const { return this->_minSlider; }

        double maxAtom() const { return this->_maxAtom; }
        double minAtom() const { return this->_minAtom; }
    
    private:
        int _maxSlider = 0;
        int _minSlider = 0;

        double _maxAtom = 0;
        double _minAtom = 0;

        void _defineMinMaxes() {
            
            auto min = this->_lines.first();
            this->_minAtom = min.x1();
            this->_minSlider = qRound(min.y1());

            auto max = this->_lines.last();
            this->_maxAtom = max.x2();
            this->_maxSlider = qRound(max.y2());

        }

        
        const QLineF& _functionToUseFromAtomValue(double atomVal) const {
            
            for(const auto &line : this->_lines) {
                auto min = line.x1();
                auto max = line.x2();
                if(atomVal >= min && atomVal <= max) {
                    return line;
                }
            }

            return this->_lines.first();

        }

        const QLineF& _functionToUseFromSliderValue(int sliderVal) const {
            
            for(const auto &line : this->_lines) {
                auto min = line.y1();
                auto max = line.y2();
                if(sliderVal >= min && sliderVal <= max) {
                    return line;
                }
            }

            return this->_lines.first();

        }

        QVector<QLineF> _lines;

};