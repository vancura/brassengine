/**
 * Module for working with floatarray data.
 * @module floatarray
 */
#include <stdint.h>

#include <lua/lua.h>
#include <lua/lauxlib.h>
#include <lua/lualib.h>

#include "float_array.h"

#include "../collections/float_array.h"

float_array_t* luaL_checkfloatarray(lua_State* L, int index) {
    float_array_t** handle = NULL;
    luaL_checktype(L, index, LUA_TUSERDATA);

    // Ensure we have correct userdata
    handle = (float_array_t**)luaL_testudata(L, index, "floatarray_nogc");
    if (!handle) {
        handle = (float_array_t**)luaL_checkudata(L, index, "floatarray");
    }

    return *handle;
}

int lua_newfloatarray(lua_State* L, size_t size) {
    float_array_t** handle = (float_array_t**)lua_newuserdata(L, sizeof(float_array_t*));
    *handle = float_array_new(size);
    luaL_setmetatable(L, "floatarray");

    return 1;
}

int lua_pushfloatarray(lua_State* L, float_array_t* array) {
    float_array_t** handle = (float_array_t**)lua_newuserdata(L, sizeof(float_array_t*));
    *handle = array;

    luaL_setmetatable(L, "floatarray_nogc");

    return 1;
}

static int float_array_gc(lua_State* L) {
    float_array_t** handle = lua_touserdata(L, 1);
    float_array_free(*handle);
    *handle = NULL;

    return 0;
}

/**
 * Returns a new floatarray
 * @function new
 * @param size Number of elements.
 * @return @{floatarray}
 */
static int module_float_array_new(lua_State* L) {
    int size = luaL_checkinteger(L, 1);

    lua_pop(L, -1);

    lua_newfloatarray(L, size);

    return 1;
}

/**
 * Adds given value to end of float array.
 * @function add
 * @param @{floatarray} Float array to modify.
 * @param float Value to add
 */
static int module_float_array_add(lua_State* L) {
    float_array_t* array = luaL_checkfloatarray(L, 1);
    float value = (float)luaL_checknumber(L, 2);

    lua_settop(L, 0);

    float_array_add(array, value);

    return 0;
}

/**
 * Resize floatarray to new length.
 * @function resize
 * @param @{floatarray} Float array to modify.
 * @param int New number of elements
 */
static int module_float_array_resize(lua_State* L) {
    float_array_t* array = luaL_checkfloatarray(L, 1);
    int size = (int)luaL_checkinteger(L, 2);

    luaL_argcheck(L, size >= 0, 2, "invalid size");

    lua_settop(L, 0);

    float_array_resize(array, size);

    return 0;
}

static int module_float_array_get(lua_State* L) {
    float_array_t* array = luaL_checkfloatarray(L, 1);
    int index = luaL_checkinteger(L, 2);

    luaL_argcheck(L, 1 <= index && index <= array->size, 2, "index out of range");

    lua_pushnumber(L, array->data[index - 1]);

    return 1;
}

static int module_float_array_set(lua_State* L) {
    float_array_t* array = luaL_checkfloatarray(L, 1);
    int index = luaL_checkinteger(L, 2);
    float value = luaL_checknumber(L, 3);

    luaL_argcheck(L, 1 <= index && index <= array->size, 2, "index out of range");

    array->data[index - 1] = value;

    return 0;
}

static int module_float_array_size(lua_State* L) {
    float_array_t* array = luaL_checkfloatarray(L, 1);

    lua_pushinteger(L, array->size);

    return 1;
}

static const struct luaL_Reg module_float_array_meta_functions[] = {
    {"__index", module_float_array_get},
    {"__newindex", module_float_array_set},
    {"__len", module_float_array_size},
    {NULL, NULL}
};

static const struct luaL_Reg module_functions[] = {
    {"new", module_float_array_new},
    {"add", module_float_array_add},
    {"resize", module_float_array_resize},
    {NULL, NULL}
};

int luaopen_floatarray(lua_State* L) {
    luaL_newlib(L, module_functions);

    // Push floatarray userdata metatable
    luaL_newmetatable(L, "floatarray");
    luaL_setfuncs(L, module_float_array_meta_functions, 0);

    lua_pushstring(L, "__gc");
    lua_pushcfunction(L, float_array_gc);
    lua_settable(L, -3);

    lua_pop(L, 1);

    // Push floatarray_nogc userdata metatable
    luaL_newmetatable(L, "floatarray_nogc");
    luaL_setfuncs(L, module_float_array_meta_functions, 0);

    lua_pop(L, 1);

    return 1;
}
