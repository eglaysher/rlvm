-- Library functions for Princess Bride.

-- Bride namespace
Bride = { }

-- Clicks in the center of the rectangle described by
function Bride:clickOnArray (base)
    left = Machine:getInt('C', base)
    width = Machine:getInt('C', base + 2)
    top = Machine:getInt('C', base + 1)
    height = Machine:getInt('C', base + 3)
    origin = Point(left + (width / 2), top + (height / 2))
    System:event():injectMouseMovement(origin)
end

function Bride:installMainMenuHandler ()
   state = 0
   World:addHandler(1, 118, function ()
      if state == 0 then
         -- The new game menu is always the intC[101] array
         Bride:clickOnArray(101)
         state = 1
      elseif state == 1 then
         System:event():injectMouseDown()
         state = 2
      elseif state == 2 then
         System:event():injectMouseUp()
         state = 3
      elseif state == 4 then
         -- array intC[241] is the Exit button
         Bride:clickOnArray(241)
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
  World:addHandler(1001, 31, function ()
    if state == 3 then
       state = 4
    end
  end)

  -- But we also have to skip selecting new game on first run.
  World:addHandler(1000, 20, function ()
                                 state = 4
                             end)
end
