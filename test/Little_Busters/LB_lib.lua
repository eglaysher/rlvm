-- Library functions for Little Busters testing.

-- namespace
LB = { }

function LB:installMainMenuHandler ()
    state = 0
    World:addHandler(9012, 370, function ()
        if state == 0 then
            origin =
                System:graphics():getChildFgObject(171, 21):getClickPointHack()
            System:event():injectMouseMovement(origin)
            state = 1
        elseif state == 1 then
            System:event():injectMouseDown()
            state = 2
        elseif state == 2 then
            System:event():injectMouseUp()
            state = 3
        elseif state == 4 then
            origin =
                System:graphics():getChildFgObject(171, 26):getClickPointHack()
            System:event():injectMouseMovement(origin)
            state = 5
        elseif state == 5 then
            System:event():injectMouseDown()
            state = 6
        elseif state == 6 then
            System:event():injectMouseUp()
            state = 7
        end
    end)

    -- Once we've started a new game, the next interaction with the main menu
    -- should quit.
    World:addHandler(513, 51, function ()
        if state == 3 then
            state = 4
        end
    end)
end

function LB:clickThroughMinigames ()
    -- Clicks during the first minigame where Rin is tossed.
    throwState = 0
    World:addHandler(9070, 290, function ()
        if throwState == 0 then
            System:event():injectMouseDown()
            throwState = 1
        elseif throwState == 1 then
            System:event():injectMouseUp()
            throwState = 2
        end
    end)

    creditState = 0
    World:addHandler(9503, 245, function ()
        if creditState == 0 then
            System:event():injectMouseDown()
            creditState = 1
        elseif creditState == 1 then
            System:event():injectMouseUp()
            creditState = 2
        end
    end)
end

function LB:installRandomDecisionHandler ()
    World:setDecisionHandler(
        function (decisions)
            if string.sub(decisions[0], 1, string.len("＃")) == "＃" then
                return decisions[0]
            else
                return nil
            end
        end)
end

function LB:handleTimedChoices (options)
    -- I hate this.
    --
    -- So SEEN9072 is a big implementation of timed choices. It randomizes the
    -- order of the options, and gives the player a short amount of time to
    -- answer, after which you fail the choice.
    --
    -- The three choices are three text object (152, {4,5,6}). To make this
    -- work, we need to scan those objects for whatever text string,
    mashingState = 0
    World:addHandler(9072, 452, function()
        g = System:graphics()
        objs = { g:getChildFgObject(152, 4),
                 g:getChildFgObject(152, 5),
                 g:getChildFgObject(152, 6) }
        for i,v in ipairs(objs) do
            if v:text() == options[1] then
                if mashingState == 0 then
                    origin = v:getClickPointHack()
                    System:event():injectMouseMovement(origin)
                    mashingState = 1
                elseif mashingState == 1 then
                    System:event():injectMouseDown()
                    mashingState = 2
                elseif mashingState == 2 then
                    System:event():injectMouseUp()
                    mashingState = 0
                end
            end
        end
    end)

    -- Removes the option in front.
    World:addHandler(9072, 749, function()
                                    table.remove(options, 1)
                                end)
end
