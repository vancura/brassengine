#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <mathc/mathc.h>

#include "../graphics.h"
#include "../log.h"
#include "draw.h"

#include "raycaster.h"

static texture_t* texture_palette[256];

typedef texture_t map_t;
typedef color_t map_data_t;

static bool map_contains(map_t* map, int x, int y) {
    if (x < 0 || x >= map->width) return false;
    if (y < 0 || y >= map->height) return false;

    return true;
}

static map_data_t map_get_data(map_t* map, int x, int y) {
    return graphics_texture_get_pixel(map, x, y);
}

static bool map_is_solid(map_t* map, int x, int y) {
    return map_get_data(map, x, y) > 0;
}

typedef struct {
    mfloat_t position[VEC2_SIZE];
    float distance;
    map_data_t data;
    bool was_vertical;
} ray_hit_info_t;

typedef struct {
    mfloat_t position[VEC2_SIZE];
    mfloat_t direction[VEC2_SIZE];
    ray_hit_info_t hit_info;
} ray_t;

static void ray_hit_info_reset(ray_hit_info_t* hit_info) {
    hit_info->distance = FLT_MAX;
    hit_info->data = 0;
    hit_info->was_vertical = false;
}

static void ray_set(ray_t* ray, mfloat_t* position, mfloat_t* direction) {
    vec2_assign(ray->position, position);
    vec2_assign(ray->direction, direction);

    vec2_assign(ray->hit_info.position, position);
    ray_hit_info_reset(&ray->hit_info);
}

static void ray_cast(ray_t* ray, map_t* map) {
    // Horizontal checks
    if (ray->direction[1] != 0.0f)
    {
        float y_intercept;
        float x_intercept;
        bool ray_facing_down = ray->direction[1] > 0;
        float distance;
        float inv_y = 1.0f / ray->direction[1];

        // Determine y-value of first horizontal intersection from ray origin
        y_intercept = floorf(ray->position[1]);
        if (ray_facing_down) {
            y_intercept += 1;
        }

        // Calculate ratio of y-components
        float ratio = (y_intercept - ray->position[1]) * inv_y;

        // Use ratio to determine x-value of first horizontal intersection from
        // ray origin
        x_intercept = (ray->direction[0] * ratio) + ray->position[0];

        // This also happens to be the distance to the first intersection
        distance = ratio;

        // Track horizontal grid intersections
        mfloat_t intersection[VEC2_SIZE];
        vec2_assign(intersection, ray->direction);
        vec2_multiply_f(intersection, intersection, ratio);
        vec2_add(intersection, intersection, ray->position);

        // After the first intersection, all following intersections are at a
        // regular interval
        mfloat_t step[VEC2_SIZE];
        vec2_assign(step, ray->direction);
        vec2_multiply_f(step, step, inv_y);
        if (!ray_facing_down) {
            vec2_multiply_f(step, step, -1.0f);
        }

        int ray_direction_offset = ray_facing_down ? 0 : -1;

        int bound = map->height;

        for(int s = 0; s < bound; s++) {
            // Ensure we are still inside map bounds
            if (!map_contains(map, intersection[0], intersection[1])) break;

            int i = floorf(intersection[0]);
            int j = floorf(intersection[1] + 0.001f) + ray_direction_offset;

            // Check if we've hit a wall
            if (map_is_solid(map, i, j)) {
                ray->hit_info.position[0] = intersection[0];
                ray->hit_info.position[1] = intersection[1];
                ray->hit_info.distance = distance;
                ray->hit_info.was_vertical = false;
                ray->hit_info.data = map_get_data(map, i, j);

                break;
            }

            // Otherwise next iteration
            vec2_add(intersection, intersection, step);
            distance += fabsf(inv_y);
        }
    }

    // Vertical checks
    if (ray->direction[0] != 0.0f)
    {
        float x_intercept;
        float y_intercept;
        bool ray_facing_right = ray->direction[0] > 0;
        float distance;
        float inv_x = 1.0f / ray->direction[0];

        // Determine x-value of first vertical intersection from ray origin
        x_intercept = floorf(ray->position[0]);
        if (ray_facing_right) {
            x_intercept += 1;
        }

        // Calculate ratio of x-components
        float ratio = (x_intercept - ray->position[0]) * inv_x;

        // Use ratio to determine y-value of first vertical intersection from
        // ray origin
        y_intercept = (ray->direction[1] * ratio) + ray->position[1];

        // This also happens to be the distance to the first intersection
        distance = ratio;

        // Track horizontal grid intersections
        mfloat_t intersection[VEC2_SIZE];
        vec2_assign(intersection, ray->direction);
        vec2_multiply_f(intersection, intersection, ratio);
        vec2_add(intersection, intersection, ray->position);

        // After the first intersection, all following intersections are at a
        // regular interval
        mfloat_t step[VEC2_SIZE];
        vec2_assign(step, ray->direction);
        vec2_multiply_f(step, step, inv_x);
        if (!ray_facing_right) {
            vec2_multiply_f(step, step, -1.0f);
        }

        int ray_direction_offset = ray_facing_right ? 0 : -1;

        int bound = map->width;

        for(int s = 0; s < bound; s++) {
            // Ensure we are still inside map bounds
            if (!map_contains(map, intersection[0], intersection[1])) break;

            // Early out if further than horizontal intersection
            if (distance > ray->hit_info.distance) break;

            int i = floorf(intersection[0] + 0.001f) + ray_direction_offset;
            int j = floorf(intersection[1]);

            // Check if we've hit a wall
            if (map_is_solid(map, i, j)) {
                ray->hit_info.position[0] = intersection[0];
                ray->hit_info.position[1] = intersection[1];
                ray->hit_info.distance = distance;
                ray->hit_info.was_vertical = true;
                ray->hit_info.data = map_get_data(map, i, j);

                break;
            }

            // Otherwise next iteration
            vec2_add(intersection, intersection, step);
            distance += fabsf(inv_x);
        }
    }
}

static void draw_wall_strip(texture_t* texture, int x, int y0, int y1, float offset) {
    int length = y1 - y0;

    const int s = texture->width * offset;
    for (int i = 0; i < length; i++) {
        float amount = i / (float)length;

        int y = y0 + i;
        int t = texture->height * amount;
        color_t c = graphics_texture_get_pixel(texture, s, t);

        graphics_set_pixel(x, y, c);
    }
}

void raycaster_render(mfloat_t* position, mfloat_t* direction, float fov, texture_t* map) {
    texture_t* render_texture = graphics_get_render_texture();

    const float width = render_texture->width;
    const float height = render_texture->height;
    const int ray_count = width;
    const float fov_rads = fov * M_PI / 180.0f;

    // Ensure direction is normalized
    vec2_normalize(direction, direction);

    /*
     * Casting rays along a plane in front of the camera.
     *
     * To determine the ray directions:
     *
     * 1. Determine distance to the projection plane. This distance will ensure
     *    that width of the projection plane bounded by our fov is the same
     *    width as the render texture.
     *
     * 2. Find left bound. This is the point on the projection plane where the
     *    left fov bound intersects the plane.
     *
     * 3. Find step vector. Because the projection plane width is the same width
     *    as the render texture, each step vector is of length one. The step
     *    vector is just the negated tangent to the camera direction.
     *
     *           \_                    _/
     * (left bound) _l<----plane---->_/
     *                \_           _/
     *                  \_   ^   _/
     *                    \_ |<---(camera direction)
     *                      \|/
     *                       c (camera position)
     */

    // 1. Determine distance to project plane
    const float distance_to_projection_plane = (width / 2.0f) / tanf(fov_rads / 2.0f);

    // 2. Find left bound

    // Calculate step vector, we need it to move along the projection plane
    mfloat_t step[VEC2_SIZE];
    vec2_tangent(step, direction);
    vec2_negative(step, step);

    // Calculate left bound
    // left_bound = position + (direction * distance_to_projection_plane) - (step * width / 2)
    mfloat_t left_bound[VEC2_SIZE];
    vec2_multiply_f(left_bound, direction, distance_to_projection_plane);
    vec2_add(left_bound, position, left_bound);
    vec2_multiply_f(step, step, width * 0.5f);
    vec2_subtract(left_bound, left_bound, step);

    // 3. Find step vector
    vec2_tangent(step, direction);
    vec2_negative(step, step);

    // Initialize ray to point to left bound
    ray_t ray;
    ray_set(&ray, position, ray.direction);
    vec2_subtract(ray.direction, left_bound, position);
    vec2_normalize(ray.direction, ray.direction);

    // Track where the next ray needs to point
    mfloat_t next[VEC2_SIZE];
    vec2_assign(next, left_bound);

    // Cast all rays
    for (int i = 0; i < ray_count; i++) {
        ray_cast(&ray, map);

        // Calculate wall height
        mfloat_t hit_vector[VEC2_SIZE];
        vec2_multiply_f(hit_vector, ray.direction, ray.hit_info.distance);
        float corrected_distance = vec2_dot(hit_vector, direction);

        float wall_height = 1.0f / corrected_distance * distance_to_projection_plane;
        float half_wall_height = wall_height / 2.0f;

        color_t c = 15;
        if (ray.hit_info.was_vertical) {
            c = 21;
        }

        // Calculate the texture normalized horizontal offset (u-coordinate).
        float offset = 0.0f;
        if (ray.hit_info.was_vertical) {
            offset = fmodf(ray.hit_info.position[1], 1.0f);

            // Flip texture to maintain correct orienation
            if (ray.direction[0] < 0) {
                offset = 1.0f - offset;
            }
        }
        else {
            offset = fmodf(ray.hit_info.position[0], 1.0f);

            // Flip texture to maintain correct orienation
            if (ray.direction[1] > 0) {
                offset = 1.0f - offset;
            }
        }

        texture_t* t = raycaster_get_texture(ray.hit_info.data);
        if (t) {
            draw_wall_strip(
                t,
                i,
                height / 2.0f - half_wall_height,
                height / 2.0f + half_wall_height,
                offset
            );
        }

        // Orient ray to next position along plane
        vec2_add(next, next, step);
        vec2_subtract(ray.direction, next, position);
        vec2_normalize(ray.direction, ray.direction);

        // Clear out hit info
        ray_hit_info_reset(&ray.hit_info);
    }
}

texture_t* raycaster_get_texture(int i) {
    return texture_palette[i];
}

void raycaster_set_texture(int i, texture_t* texture) {
    texture_palette[i] = texture;
}
