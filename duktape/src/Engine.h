#ifndef __ENGINE_H__
#define __ENGINE_H__

#include <dmsdk/sdk.h>
#include "Script.h"

class Engine
{
private:
    static Engine *instance;

    Engine(){};
    ~Engine(){};

public:
    static Engine *getInstance();

    Script *load(lua_State *L);
};

#endif // __ENGINE_H__