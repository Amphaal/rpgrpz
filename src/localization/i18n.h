#pragma once

#include <QLocale>
#include <string>

#include "i18n/en_EN.hpp"
#include "i18n/fr_FR.hpp"

#include "i18n/Ii18n.h"

class I18n {
     public:
        static ITranslator* tr();

        static QString getLocaleName();

    private:

        I18n();  // Private so that it can  not be called
        I18n(I18n const&);             // copy constructor is private
        //I18n& operator=(I18n const&){};  // assignment operator is private

        static inline ITranslator* m_pInstance = nullptr;

        static ITranslator* setTranslator(); 
};