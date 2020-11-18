#ifndef __SCRIPT_H__
#define __SCRIPT_H__

#include <dmsdk/sdk.h>
#include "duktape.h"

struct Script
{
    uint32_t id;
    void push(lua_State *L);
};

#endif // __SCRIPT_H__