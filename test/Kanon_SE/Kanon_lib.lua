-- Library functions for Kanon.

-- Kanon namespace
Kanon = { }

-- TODOs:
-- * Need to generate a click when "Little Fragments" starts playing. Does this
--   happen with each ending?
-- * Returning to the main menu at the end is a blank screen... ???

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
