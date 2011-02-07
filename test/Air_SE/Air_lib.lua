-- Library functions for AIR.

-- Air namespace
Air = { }

function Air:mashClickWithState(state)
    if state == 0 then
        System:event():injectMouseDown()
        state = 1
    else
        System:event():injectMouseUp()
        state = 0
    end
end

function Air:installMainMenuHandler (gameType)
    state = 0
    -- Main menu navigation
    World:addHandler(9032, 470, function ()
        if state == 0 then
            -- Object 20 is always the new game button.
            origin = System:graphics():getFgObject(20):getClickPointHack()
            System:event():injectMouseMovement(origin)
            state = 1
        elseif state == 1 then
            System:event():injectMouseDown()
            state = 2
        elseif state == 2 then
            System:event():injectMouseUp()
            state = 3
        elseif state == 6 then
            -- Object 24 is the Exit button
            origin = System:graphics():getFgObject(24):getClickPointHack()
            System:event():injectMouseMovement(origin)
            state = 7
        elseif state == 7 then
            System:event():injectMouseDown()
            state = 8
        elseif state == 8 then
            System:event():injectMouseUp()
            state = 9
        end
    end)

    -- Game select navigation
    World:addHandler(9038, 163, function ()
        if state == 3 then
            if gameType == "DREAM" then
                objNum = 20
            elseif gameType == "SUMMER" then
                objNum = 21
            elseif gameType == "AIR" then
                objNum = 22
            else
                print("Invalid menu given to Air:installMainMenuHandler()");
                print("Assuming DREAM.");
                objNum = 20
            end
            origin = System:graphics():getFgObject(objNum):getClickPointHack()
            System:event():injectMouseMovement(origin)
            state = 4
        elseif state == 4 then
            System:event():injectMouseDown()
            state = 5
        elseif state == 5 then
            System:event():injectMouseUp()
            state = 6
        end
    end)
end
