#pragma once

#include "src/shared/models/RPZUser.h"

class Authorisations {
    public:        
        static void resetHostAbility() {
            _isHostAble = true;
        }

        static bool isHostAble()  {
            return _isHostAble;
        }

        static void defineHostAbility(const RPZUser &user) {
            _isHostAble = user.role() == RPZUser::Role::Host;
        }

    private:
        static inline bool _isHostAble = true;

};