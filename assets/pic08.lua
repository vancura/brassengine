local pico = require("apis/pico")

frame = 0

function t()
    frame = frame + 0.0001
    return frame
end

function all(arr)
    local i = 0
    local n = #arr
    return function()
        i = i + 1
        if i <=n then
            return arr[i]
        end
    end
end

function add(t, i)
    table.insert(t, i)
end

function _init()
    -- make some points
    -- 7x7x7 cube

    setup_palette()
    --pico.clip(0, 0, 128, 128)

    pt={}
    for z = -1, 1, 1 / 3 do
        for y = -1, 1, 1 / 3 do
            for x = -1, 1, 1 / 3 do
                p = {}
                p.x, p.y, p.z = x, y, -z
                p.col = 8 + (x * 2 + y * 3) % 8
                add(pt, p)
            end
        end
    end
end

--rotate x,y by angle a
function rot(x, y, a)
    local x0 = x
    x = math.cos(a) * x - math.sin(a) * y
    y = math.cos(a) * y + math.sin(a) * x0

    return x, y
end

function _draw()
    pico.cls()
    for p in all(pt) do
        -->camera space
        p.cx, p.cz = rot(p.x, p.z, t() / 8)
        p.cy, p.cz = rot(p.y, p.cz, t() / 7)

        p.cz = p.cz + 2 + math.cos(t()/6)

    end

    --sort by distance from camera
    --because they go out of order
    --slowly, doing a bubble sort
    --up and down 3 times is good
    --enough

    for pass = 1, 3 do
        for i = 1, #pt - 1 do
            if pt[i].cz < pt[i + 1].cz then
                pt[i],pt[i + 1] = pt[i + 1], pt[i]
            end
        end
        for i = #pt - 1, 1, -1 do
            if pt[i].cz < pt[i + 1].cz then
                pt[i], pt[i + 1] = pt[i + 1], pt[i]
            end
        end
    end

    rad1 = 5 + math.cos(t() / 4) * 4

    for p in all(pt) do
        --> screen space
        sx = 160 + p.cx * 64 / p.cz
        sy = 100 + p.cy * 64 / p.cz
        rad = rad1 / p.cz

        if (p.cz > 0.1) then -- clip
            pico.circfill(sx, sy, rad, p.col)
            pico.circfill(sx + rad / 3, sy - rad / 3,
            rad / 3, 7)
        end
    end
end

function setup_palette()
    black = 0
    dark_blue = 1
    dark_purple = 2
    dark_green = 3
    brown = 4
    dark_gray = 5
    light_ray = 6
    white = 7
    red = 8
    orange = 9
    yellow = 10
    green = 11
    blue = 12
    indigo = 13
    pink = 14
    peach = 15

    -- pico-8 palette
    palette(black, 0, 0, 0)
    palette(dark_blue, 29, 43, 83)
    palette(dark_purple, 126, 37, 83)
    palette(dark_green, 0, 135, 81)
    palette(brown, 171, 82, 54)
    palette(dark_gray, 95, 87, 79)
    palette(light_ray, 194, 195, 199)
    palette(white, 255, 241, 232)
    palette(red, 255, 0, 77)
    palette(orange, 255, 163, 0)
    palette(yellow, 255, 236, 39)
    palette(green, 0, 228, 54)
    palette(blue, 41, 173, 255)
    palette(indigo, 131, 118, 156)
    palette(pink, 255, 119, 168)
    palette(peach, 255, 204, 170)
end