#include "i18n.h"

ITranslator* I18n::tr() {   
    if (!m_pInstance) {
        m_pInstance = setTranslator();
    }
    return m_pInstance;
}

QString I18n::getLocaleName() {
    return QLocale::system().name();
}

I18n::I18n(){};

I18n::I18n(I18n const&){}; 

ITranslator* I18n::setTranslator() {
    auto lName = I18n::getLocaleName();

    if (lName.mid(0,2) == "fr") {
        return new Translator_FR();
    } else {
        return new Translator_EN();
    }
} 
