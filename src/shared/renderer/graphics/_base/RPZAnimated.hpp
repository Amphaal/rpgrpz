#pragma once

class RPZAnimated {
    public:
        virtual void triggerAnimation() = 0; //only to be used on main thread !
};