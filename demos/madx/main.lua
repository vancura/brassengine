-- Testing blitting stuff
local draw = require("draw")
local mouse = require("input.mouse")
local Map = require("map")
local generate = require("generator")

-- Called once at startup
function _init()
    print("lua init")

    p_x = 0
    p_y = 0

    button_states = {}

    map = Map.new(36, 21)
    generate:Noise(map)
    generate:Threshold(map, 0.85)
end

function handle_buttons()
    for i = 0, 5 do
        if button(i) then
            button_states[i] = button_states[i] + 1
        else
            button_states[i] = 0
        end
    end
end

function buttonp(index)
    local state = button_states[index]

    if state == 1 or state == 16 then
        return true
    end

    if state > 16 and (state - 16) % 4 == 0 then
        return true
    end

    return false
end

-- Called once per frame
function _update()
    handle_buttons()

    local n_x, n_y = p_x, p_y

    if buttonp(0) then
        n_x = p_x - 1
    end

    if buttonp(1) then
        n_x = p_x + 1
    end

    if buttonp(2) then
        n_y = p_y - 1
    end

    if buttonp(3) then
        n_y = p_y + 1
    end

    if map:Contains(n_x, n_y) and not map:IsSolid(n_x, n_y) then
        p_x, p_y = n_x, n_y
    end
end

-- Called once per frame
function _draw()
    draw.clear(0)
    local x, y = mouse.position()

    frame("ENTRANCE")
    draw_map()
    grid()
    player()
end

function sprite(index, x, y)
    local sx = (index % 32) * 8
    local sy = (index // 32) * 8
    draw.test_blit(sx, sy, 8, 8, x, y, 8, 8)
end

function frame(name)
    -- Draw name
    draw.text(name, 16, 8)

    local s = 398

    -- Draw top + bottom frame
    for x = 2, (320 - 24) // 8 do
        if x > #name + 2 then
            sprite(s, x * 8, 8)
        end

        sprite(s, x * 8, 200 - 16)
    end

    -- Draw left + right frame
    for y = 2, (200 - 24) // 8 do
        if y > 1 then
            sprite(s + 32, 8, y * 8)
        end

        sprite(s + 32, 320 - 16, y * 8)
    end

    -- Draw corners
    sprite(s + 62, 8, 8)
    sprite(s + 63, 320 - 16, 8)
    sprite(s + 63 + 31, 8, 200 - 16)
    sprite(s + 63 + 32, 320 - 16, 200 - 16)
end

function grid()
    for y = 2, (200 // 8) - 3 do
        for x = 2, (320 // 8) - 3 do
            draw.rectangle(x * 8, y * 8, 8, 8, 4)
        end
    end
end

function player()
    local x, y = p_x * 8 + 16, p_y * 8 + 16
    sprite(62, x, y)
end

function draw_map()
    for y = 1, map.height - 1 do
        for x = 1, map.width - 1 do
            local d = map:GetData(x, y)

            if d == 1 then
                sprite(50, x * 8 + 16, y * 8 + 16)
            end
        end
    end
end