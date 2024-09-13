/**
 * Module for working with texture data.
 * @module texture
 */
#include <stdbool.h>
#include <string.h>

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

texture_t* luaL_opttexture(lua_State* L, int index, texture_t* default_) {
    if (lua_isnoneornil(L, 2)) return default_;

    texture_t** handle = NULL;
    luaL_checktype(L, index, LUA_TUSERDATA);

    // Ensure we have correct userdata
    handle = (texture_t**)luaL_testudata(L, index, "texture_nogc");
    if (!handle) {
        handle = (texture_t**)luaL_testudata(L, index, "texture");
    }

    if (!handle) {
        return default_;
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

static int modules_texture_meta_index(lua_State* L) {
    texture_t* texture = luaL_checktexture(L, 1);
    const char* key = luaL_checkstring(L, 2);

    lua_settop(L, 0);

    if (strcmp(key, "pixels") == 0) {
        lua_newtable(L);

        for (int i = 0; i < texture->width * texture->height; i++) {
            lua_pushinteger(L, i + 1);
            lua_pushinteger(L, texture->pixels[i]);
            lua_settable(L, -3);
        }
    }
    else if (strcmp(key, "width") == 0) {
        lua_pushinteger(L, texture->width);
    }
    else if (strcmp(key, "height") == 0) {
        lua_pushinteger(L, texture->height);
    }
    else {
        // Check module fields. This enables usage of the colon operator.
        luaL_requiref(L, "texture", NULL, false);
        if (lua_type(L, -1) == LUA_TTABLE) {
            lua_getfield(L, -1, key);
        }
        else {
            lua_pushnil(L);
        }
    }

    return 1;
}

static int modules_texture_meta_newindex(lua_State* L) {
    texture_t* texture = luaL_checktexture(L, 1);
    const char* key = luaL_checkstring(L, 2);

    if (strcmp(key, "pixels") == 0) {
        size_t pixel_count = texture->width * texture->height;
        size_t table_size = lua_rawlen(L, 3);

        if (table_size == pixel_count) {
            for (int i = 0; i < pixel_count; i++) {
                int index = i + 1;
                lua_pushinteger(L, index);
                lua_gettable(L, 3);

                texture->pixels[i] = (int)luaL_checknumber(L, -1);

                lua_pop(L, 1);
            }

            lua_settop(L, 0);
        }
        else {
            luaL_error(L, "BAD!");
        }
    }
    else {
        luaL_error(L, "attempt to index a texture value");
    }

    return 0;
}

static const struct luaL_Reg modules_texture_meta_functions[] = {
    {"__index", modules_texture_meta_index},
    {"__newindex", modules_texture_meta_newindex},
    {NULL, NULL}
};

/**
 * Create new texture.
 * @function new
 * @tparam integer width Texture width
 * @tparam integer height Texture height
 * @treturn texture
 */
static int modules_texture_new(lua_State* L) {
    int width = (int)luaL_checknumber(L, 1);
    int height = (int)luaL_checknumber(L, 2);

    lua_pop(L, -1);

    lua_newtexture(L, width, height);

    return 1;
}

/**
 * @type texture
 */

/**
 * Returns a copy of this texture.
 * @function copy
 * @treturn texture
 */
static int modules_texture_copy(lua_State* L) {
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
 * @tparam integer color Fill color
 */
static int modules_texture_clear(lua_State* L) {
    texture_t* texture = luaL_checktexture(L, 1);
    int color = (int)luaL_checknumber(L, 2);

    lua_pop(L, -1);

    graphics_texture_clear(texture, color);

    return 0;
}

/**
 * Sets pixel at given position and color.
 * @function set_pixel
 * @tparam integer x Pixel x-coordinate
 * @tparam integer y Pixel y-coordinate
 * @tparam integer color Pixel color
 */
static int modules_texture_pixel_set(lua_State* L) {
    texture_t* texture = luaL_checktexture(L, 1);
    int x = (int)luaL_checknumber(L, 2);
    int y = (int)luaL_checknumber(L, 3);
    int color = (int)luaL_checknumber(L, 4);

    lua_pop(L, -1);

    graphics_texture_pixel_set(texture, x, y, color);

    return 0;
}

/**
 * Gets pixel at given position.
 * @function get_pixel
 * @tparam integer x Pixel x-coordinate
 * @tparam integer y Pixel y-coordinate
 * @treturn integer Pixel color
 */
static int modules_texture_pixel_get(lua_State* L) {
    texture_t* texture = luaL_checktexture(L, 1);
    int x = (int)luaL_checknumber(L, 2);
    int y = (int)luaL_checknumber(L, 3);

    lua_pop(L, -1);

    color_t color = graphics_texture_pixel_get(texture, x, y);
    lua_pushinteger(L, color);

    return 1;
}

/**
 * Copy given source texture to this texture with given offset.
 * @function blit
 * @tparam texture.texture source Texture to copy from
 * @tparam integer x Destination x-offset
 * @tparam integer y Destination y-offset
 */
static int modules_texture_blit(lua_State* L) {
    texture_t* dest = luaL_checktexture(L, 1);
    texture_t* source = luaL_checktexture(L, 2);
    int x = (int)luaL_checknumber(L, 3);
    int y = (int)luaL_checknumber(L, 4);

    rect_t drect = {x, y, source->width, source->height};

    graphics_texture_blit(source, dest, NULL, &drect);

    return 0;
}

/**
 * An array copy of pixel indices.
 * @tfield {integer,...} pixels
 */

/**
 * Texture width in pixels.
 * @tfield integer width (read-only)
 */

/**
 * Texture height in pixels.
 * @tfield integer height (read-only)
 */

static const struct luaL_Reg modules_texture_functions[] = {
    {"new", modules_texture_new},
    {"copy", modules_texture_copy},
    {"clear", modules_texture_clear},
    {"set_pixel", modules_texture_pixel_set},
    {"get_pixel", modules_texture_pixel_get},
    {"blit", modules_texture_blit},
    {NULL, NULL}
};

int luaopen_texture(lua_State* L) {
    luaL_newlib(L, modules_texture_functions);

    // Push texture userdata metatable
    luaL_newmetatable(L, "texture");
    luaL_setfuncs(L, modules_texture_meta_functions, 0);

    lua_pushstring(L, "__gc");
    lua_pushcfunction(L, texture_gc);
    lua_settable(L, -3);

    lua_pop(L, 1);

    // Push texture_nogc userdata metatable
    luaL_newmetatable(L, "texture_nogc");
    luaL_setfuncs(L, modules_texture_meta_functions, 0);

    lua_pop(L, 1);

    return 1;
}
