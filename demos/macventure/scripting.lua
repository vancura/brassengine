local scripting = {}

local sandbox = {}

local entrance_intro_text_shown = false

function sandbox.entrance_enter()
    if not entrance_intro_text_shown then
        game_screen:describe("The last thing that you remember is standing before the wizard Lakmir as he waved his hands. Now you find yourself staring at an entryway which lies at the edge of a forest. The Druid's words ring in your ears: \"Within the castle Shadowgate lies your quest. The dreaded warlock lord will use his black magic to raise the behemoth from the dark depths. The combination of his evil arts and the great titan's power will surely destroy us all! You are the last of the line of kings, the seed of prophecy that was foretold eons ago. Only you can stop the evil one from darkening our world forever! Fare thee well.\" Gritting your teeth, you swear by your god's name that you will destroy the warlock lord!")
        entrance_intro_text_shown = true
        return
    end

    game_screen:describe("It's the entrance to Shadowgate. You can hear wolves howling deep in the forest behind you...")
end

function sandbox.entrance_skull_interact()
end

function sandbox.entrance_skull_inspect()
    game_screen:describe("It's the skull of some creature. Its meaning is quite clear: death lurks inside.")
end

function sandbox.entrance_key_interact()
end

function sandbox.entrance_key_inspect()
    game_screen:describe("It's a small iron key.")
end

function sandbox.entrance_door_interact()
end

function sandbox.entrance_door_inspect()
    game_screen:describe("It's a heavy wooden door with iron hinges.")
end

local hallway_intro_text_shown = false

function sandbox.hallway_enter()
    if not hallway_intro_text_shown then
        game_screen:describe("\"That pitiful wizard Lakmir was a fool to send a buffoon like you to stop me. You will surely regret it for the only thing here for you is a horrible death!\" The sound of maniacal laughter echoes in your ears.")
        hallway_intro_text_shown = true
        return
    end
    
    game_screen:describe("You stand in a long corridor. Huge stone archways line the entire hall.")
end

function sandbox.hallway_exit_locked()
    game_screen:describe("It's locked.")
end

function sandbox.go_to(room_id)
    game_screen:set_room(room_id)
end

function scripting.execute(s)
    if s == nil then
        return
    end

    local func = load(s, nil, nil, sandbox)

    if func == nil then
        print("Failed to execute script: "..s)
        return
    end

    func()
end

return scripting
