-- Library functions for CLANNAD. 

-- Declare the CLANNAD function's namespace
CLANNAD = { }

function CLANNAD:installMainMenuHandler ()
  -- TODO: Handle exiting the game once we're done with it.
  state = 0
  World:addHandler(9032, 944, function ()
    -- Object 20 is the New Game button
    if state == 0 then
      origin = System:graphics():getFgObject(20):getClickPointHack()
      System:event():injectMouseMovement(origin)
      state = 1
    elseif state == 1 then
      System:event():injectMouseDown()
      state = 2
    elseif state == 2 then
      System:event():injectMouseUp()
      state = 3
    end
  end)

  -- Once we've started a New Game, we shouldn't see the main menu until we are
  -- ready to exit.

  --World:addHandler(
end

-- Some paths have Sunohara and other characters being throw around... and will
-- continue until a click breaks them free.
function CLANNAD:clickOnCharactersBeingThrown ()
  -- Sunohara
  World:addHandler(1, 137, function ()
    System:event():injectMouseDown()
  end)
end
