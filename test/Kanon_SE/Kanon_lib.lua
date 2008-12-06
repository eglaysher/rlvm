-- Library functions for Kanon.

-- Kanon namespace
Kanon = { }

function Kanon:mashClickWithState(state)
    if state == 0 then
        System:event():injectMouseDown()
        state = 1
    else
        System:event():injectMouseUp()
        state = 0
    end
end

function Kanon:installMainMenuHandler ()
   state = 0
   World:addHandler(9032, 396, function ()
      if state == 0 then
         -- Object 10 is the New Game button
         origin = System:graphics():getFgObject(10):getClickPointHack()
         System:event():injectMouseMovement(origin)
         state = 1
      elseif state == 1 then
         System:event():injectMouseDown()
         state = 2
      elseif state == 2 then
         System:event():injectMouseUp()
         state = 3
      elseif state == 4 then
         -- Object 14 is the Exit button
         origin = System:graphics():getFgObject(14):getClickPointHack()
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

  -- Once we've started a New Game, we shouldn't see the main menu until we are
  -- ready to exit.
  World:addHandler(50, 17, function ()
    if state == 3 then
       state = 4
    end
  end)
end

function Kanon:skipEverything ()
    state = 0

    -- Skips the date display
    World:addHandler(9070, 52, function() Kanon:mashClickWithState(state) end)

    -- Skips black on white text (everything but the last one)
    World:addHandler(8501, 162, function() Kanon:mashClickWithState(state) end)

    -- Inject the click after the end credits
    World:addHandler(8504, 315, function() Kanon:mashClickWithState(state) end)
end
