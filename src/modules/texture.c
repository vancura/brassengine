/**
 * Module for working with texture data.
 * @module texture
 */
#include <stdbool.h>

#include <lua/lua.h>
#include <lua/lauxlib.h>
#include <lua/lualib.h>

#include "texture.h"

#include "../assets.h"
#include "../graphics.h"

texture_t* luaL_checktexture(lua_State* L, int index) {
    texture_t** handle = NULL;
    luaL_checktype(L, index, LUA_TUSERDATA);

    // Ensure we have correct userdata
    handle = (texture_t**)luaL_testudata(L, index, "texture_nogc");
    if (!handle) {
        handle = (texture_t**)luaL_checkudata(L, index, "texture");
    }

    return *handle;
}

int lua_newtexture(lua_State* L, int width, int height) {
    texture_t** handle = (texture_t**)lua_newuserdata(L, sizeof(texture_t*));
    *handle = graphics_texture_new(width, height, NULL);
    luaL_setmetatable(L, "texture");

    return 1;
}

int lua_pushtexture(lua_State* L, texture_t* texture) {
    texture_t** handle = (texture_t**)lua_newuserdata(L, sizeof(texture_t*));
    *handle = texture;
    luaL_setmetatable(L, "texture_nogc");

    return 1;
}

static int texture_gc(lua_State* L) {
    texture_t** texture = lua_touserdata(L, 1);
    graphics_texture_free(*texture);
    *texture = NULL;

    return 0;
}

static int texture_size(lua_State* L) {
    texture_t* texture = luaL_checktexture(L, 1);

    lua_pushinteger(L, texture->width);
    lua_pushinteger(L, texture->height);

    return 2;
}

static const struct luaL_Reg texture_methods[] = {
    {"size", texture_size},
    {NULL, NULL}
};

/**
 * Create new texture.
 * @function new
 * @param width Texture width
 * @param height Texture height
 * @return Texture userdata
 */
static int bindings_texture_new(lua_State* L) {
    int width = (int)luaL_checknumber(L, 1);
    int height = (int)luaL_checknumber(L, 2);

    lua_pop(L, -1);

    lua_newtexture(L, width, height);

    return 1;
}

/**
 * Copy given texture.
 * @function copy
 * @param texture Texture to copy
 * @return Texture userdata
 */
static int bindings_texture_copy(lua_State* L) {
    texture_t* source = luaL_checktexture(L, 1);

    lua_pop(L, -1);

    texture_t** handle = (texture_t**)lua_newuserdata(L, sizeof(texture_t*));
    *handle = graphics_texture_copy(source);
    luaL_setmetatable(L, "texture");

    return 1;
}

/**
 * Fill entire texture with color.
 * @function clear
 * @param texture Texture userdata
 * @param color Fill color
 */
static int bindings_texture_clear(lua_State* L) {
    texture_t* texture = luaL_checktexture(L, 1);
    int color = (int)luaL_checknumber(L, 2);

    lua_pop(L, -1);

    graphics_texture_clear(texture, color);

    return 0;
}

/**
 * Draw a pixel at given position and color.
 * @function set_pixel
 * @param texture Texture userdata
 * @param x Pixel x-coordinate
 * @param y Pixel y-coordinate
 * @param color Pixel color
 */
static int bindings_texture_set_pixel(lua_State* L) {
    texture_t* texture = luaL_checktexture(L, 1);
    int x = (int)luaL_checknumber(L, 2);
    int y = (int)luaL_checknumber(L, 3);
    int color = (int)luaL_checknumber(L, 4);

    lua_pop(L, -1);

    graphics_texture_set_pixel(texture, x, y, color);

    return 0;
}

/**
 * Gets pixel at given position.
 * @function get_pixel
 * @param texture Texture userdata
 * @param x Pixel x-coordinate
 * @param y Pixel y-coordinate
 */
static int bindings_texture_get_pixel(lua_State* L) {
    texture_t* texture = luaL_checktexture(L, 1);
    int x = (int)luaL_checknumber(L, 2);
    int y = (int)luaL_checknumber(L, 3);

    lua_pop(L, -1);

    color_t color = graphics_texture_get_pixel(texture, x, y);
    lua_pushinteger(L, color);

    return 1;
}

/**
 * Copy a portion of one texture to another.
 * @function blit
 * @param source_texture Texture to copy from
 * @param destination_texture Texture to copy to
 * @param x Destination x-offset
 * @param y Destination y-offset
 */
static int bindings_texture_blit(lua_State* L) {
    texture_t* source = luaL_checktexture(L, 1);
    texture_t* dest = luaL_checktexture(L, 2);
    int x = (int)luaL_checknumber(L, 3);
    int y = (int)luaL_checknumber(L, 4);

    rect_t drect = {x, y, source->width, source->height};

    graphics_texture_blit(source, dest, NULL, &drect);

    return 0;
}

static const struct luaL_Reg module_functions[] = {
    {"new", bindings_texture_new},
    {"copy", bindings_texture_copy},
    {"clear", bindings_texture_clear},
    {"set_pixel", bindings_texture_set_pixel},
    {"get_pixel", bindings_texture_get_pixel},
    {"blit", bindings_texture_blit},
    {NULL, NULL}
};

int luaopen_texture(lua_State* L) {
    luaL_newlib(L, module_functions);

    // Push texture userdata metatable
    luaL_newmetatable(L, "texture");
    luaL_newlib(L, texture_methods);
    lua_setfield(L, -2, "__index");

    lua_pushstring(L, "__gc");
    lua_pushcfunction(L, texture_gc);
    lua_settable(L, -3);

    lua_pop(L, 1);

    // Push texture_nogc userdata metatable
    luaL_newmetatable(L, "texture_nogc");
    luaL_newlib(L, texture_methods);
    lua_setfield(L, -2, "__index");

    lua_pop(L, 1);

    return 1;
}
