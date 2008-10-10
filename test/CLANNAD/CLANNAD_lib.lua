-- Library functions for CLANNAD. 

-- Declare the CLANNAD function's namespace
CLANNAD = { }

function CLANNAD:installMainMenuHandler ()
  state = 0
  World:addHandler(9032, 944, function ()
    if state == 0 then
      -- Object 20 is the New Game button
      origin = System:graphics():getFgObject(20):getClickPointHack()
      System:event():injectMouseMovement(origin)
      state = 1
    elseif state == 1 then
      System:event():injectMouseDown()
      state = 2
    elseif state == 2 then
      System:event():injectMouseUp()
      state = 3
    elseif state == 4 then
      -- Object 27 is the Exit button
      origin = System:graphics():getFgObject(27):getClickPointHack()
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
  World:addHandler(6900, 17, function ()
    if state == 3 then
       state = 4
    end
  end)
end

-- Some paths have Sunohara and other characters being throw around... and will
-- continue until a click breaks them free.
function CLANNAD:clickOnCharactersBeingThrown ()
  -- Sunohara
  World:addHandler(1, 137, function ()
    System:event():injectMouseDown()
  end)
end
