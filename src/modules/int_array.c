/**
 * Module for working with intarray data.
 * @module intarray
 */
#include <stdint.h>

#include <lua/lua.h>
#include <lua/lauxlib.h>
#include <lua/lualib.h>

#include "int_array.h"

#include "../collections/int_array.h"

int_array_t* luaL_checkintarray(lua_State* L, int index) {
    int_array_t** handle = NULL;
    luaL_checktype(L, index, LUA_TUSERDATA);

    // Ensure we have correct userdata
    handle = (int_array_t**)luaL_testudata(L, index, "intarray_nogc");
    if (!handle) {
        handle = (int_array_t**)luaL_checkudata(L, index, "intarray");
    }

    return *handle;
}

int lua_newintarray(lua_State* L, size_t size) {
    int_array_t** handle = (int_array_t**)lua_newuserdata(L, sizeof(int_array_t*));
    *handle = int_array_new(size);
    luaL_setmetatable(L, "intarray");

    return 1;
}

int lua_pushintarray(lua_State* L, int_array_t* array) {
    int_array_t** handle = (int_array_t**)lua_newuserdata(L, sizeof(int_array_t*));
    *handle = array;

    luaL_setmetatable(L, "intarray_nogc");

    return 1;
}

static int int_array_gc(lua_State* L) {
    int_array_t** handle = lua_touserdata(L, 1);
    int_array_free(*handle);
    *handle = NULL;

    return 0;
}

/**
 * Returns a new intarray
 * @function new
 * @param size Number of elements.
 * @return @{intarray}
 */
static int module_int_array_new(lua_State* L) {
    int size = luaL_checkinteger(L, 1);

    lua_pop(L, -1);

    lua_newintarray(L, size);

    return 1;
}

/**
 * Adds given value to end of int array.
 * @function add
 * @param @{intarray} int array to modify.
 * @param int Value to add
 */
static int module_int_array_add(lua_State* L) {
    int_array_t* array = luaL_checkintarray(L, 1);
    int value = (int)luaL_checknumber(L, 2);

    lua_settop(L, 0);

    int_array_add(array, value);

    return 0;
}

/**
 * Resize intarray to new length.
 * @function resize
 * @param @{intarray} int array to modify.
 * @param int New number of elements
 */
static int module_int_array_resize(lua_State* L) {
    int_array_t* array = luaL_checkintarray(L, 1);
    int size = (int)luaL_checkinteger(L, 2);

    luaL_argcheck(L, size >= 0, 2, "invalid size");

    lua_settop(L, 0);

    int_array_resize(array, size);

    return 0;
}

static int module_int_array_get(lua_State* L) {
    int_array_t* array = luaL_checkintarray(L, 1);
    int index = luaL_checkinteger(L, 2);

    luaL_argcheck(L, 1 <= index && index <= array->size, 2, "index out of range");

    lua_pushnumber(L, array->data[index - 1]);

    return 1;
}

static int module_int_array_set(lua_State* L) {
    int_array_t* array = luaL_checkintarray(L, 1);
    int index = luaL_checkinteger(L, 2);
    int value = luaL_checknumber(L, 3);

    luaL_argcheck(L, 1 <= index && index <= array->size, 2, "index out of range");

    array->data[index - 1] = value;

    return 0;
}

static int module_int_array_size(lua_State* L) {
    int_array_t* array = luaL_checkintarray(L, 1);

    lua_pushinteger(L, array->size);

    return 1;
}

static const struct luaL_Reg module_int_array_meta_functions[] = {
    {"__index", module_int_array_get},
    {"__newindex", module_int_array_set},
    {"__len", module_int_array_size},
    {NULL, NULL}
};

static const struct luaL_Reg module_functions[] = {
    {"new", module_int_array_new},
    {"add", module_int_array_add},
    {"resize", module_int_array_resize},
    {NULL, NULL}
};

int luaopen_intarray(lua_State* L) {
    luaL_newlib(L, module_functions);

    // Push intarray userdata metatable
    luaL_newmetatable(L, "intarray");
    luaL_setfuncs(L, module_int_array_meta_functions, 0);

    lua_pushstring(L, "__gc");
    lua_pushcfunction(L, int_array_gc);
    lua_settable(L, -3);

    lua_pop(L, 1);

    // Push intarray_nogc userdata metatable
    luaL_newmetatable(L, "intarray_nogc");
    luaL_setfuncs(L, module_int_array_meta_functions, 0);

    lua_pop(L, 1);

    return 1;
}
