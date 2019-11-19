#pragma once

#include <QVector>

class CrossEquities {
    public:
        struct CrossEquity {
            double atomValue;
            int sliderValue;
        };

        CrossEquities(const QVector<CrossEquity> &equities) : _equities(equities) {
            
            Q_ASSERT(equities.count() > 1);

            for(auto const & equity : equities) {
                this->_atomEquities += equity.atomValue;
                this->_sliderEquities += equity.sliderValue;
            }

            this->_limit = this->_atomEquities.count() - 1; //prevent iterating through last pair
        }

        double toAtomValue(int sliderVal) const {
            
            int lowSliderBound = this->_sliderEquities.at(0);
            double lowAtomBound = this->_atomEquities.at(0);

            auto index = 1;
            while(true) {
                
                if(index >= this->_limit) break; //if limit is reached, skip

                auto s = this->_sliderEquities.at(index);

                if(s > sliderVal) break; //if this equity is bigger than searched value, skip

                lowSliderBound = s;
                lowAtomBound = this->_atomEquities.at(index);

                index++;

            }

            auto highSliderBound = this->_sliderEquities.at(index);

            //cross multiply
            return (lowAtomBound * highSliderBound) / lowSliderBound;

        }

        int toSliderValue(double atomVal) const {
            
            int lowSliderBound = this->_sliderEquities.at(0);
            double lowAtomBound = this->_atomEquities.at(0);

            auto index = 1;
            while(true) {
                
                if(index >= this->_limit) break; //if limit is reached, skip

                auto a = this->_atomEquities.at(index);

                if(a > atomVal) break; //if this equity is bigger than searched value, skip

                lowSliderBound = this->_sliderEquities.at(index);
                lowAtomBound = a;

                index++;

            }

            auto highAtomBound = this->_atomEquities.at(index);

            //cross multiply
            return (lowSliderBound * highAtomBound) / lowAtomBound;

        }

        const QVector<CrossEquity>& v() const {
            return this->_equities;
        }
    
    private:
        QVector<double> _atomEquities;
        QVector<int> _sliderEquities;
        int _limit;

        QVector<CrossEquity> _equities;

};