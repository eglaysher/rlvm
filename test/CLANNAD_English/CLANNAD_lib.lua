-- Library functions for CLANNAD.

-- Declare the CLANNAD function's namespace
CLANNAD = { }

AfterStoryLights = {
    [1] = "Nagisa",
    [2] = "Fuuko",
    [3] = "Tomoyo",
    [4] = "Kyou",
    [5] = "Kotomi",
    [6] = "Yukine",
    [9] = "Mei",
    [12] = "Kappei",
    [13] = "Misae",
    [15] = "Koumura",
    [30] = "Yukine (?)",
    [31] = "Tomoyo (?)"
}

TrueEndLights = {
    [14] = "Naoyuki Okazaki",
    [33] = "Thirty-three (6802)",
    [10] = "Ten (6802)",
    [8]  = "Sanae",
    [11] = "Yuusuke",
    [7]  = "Akio"
}

-- Returns a string array of all routes still needed to open the After Story
function CLANNAD:routesNeededFor (mapping)
   routes = { }

   for key,value in pairs(mapping) do
      if Machine:getInt('G', key) == 0 then
         table.insert(routes, value)
      end
   end

   return routes
end

function CLANNAD:installMainMenuHandler (type)
  state = 0
  World:addHandler(9032, 1035, function ()
    if state == 0 then
      if type == "New Game" then
        -- Object 20 is the New Game button
        origin = System:graphics():getFgObject(20):getClickPointHack()
      elseif type == "After Story" then
        -- Object 22 is the After Story button
        obj = System:graphics():getFgObject(22)

        if obj:visible() == 0 then
           paths = CLANNAD:routesNeededFor(AfterStoryLights)
          routes = table.concat(paths, ", ")
          errmsg = "After Story not unlocked yet! Still need: " .. routes
          World:error(errmsg)
        end

        origin = obj:getClickPointHack()
      end

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
  if type == "New Game" then
     World:addHandler(6900, 17, function ()
       if state == 3 then
         state = 4
       end
    end)
  elseif type == "After Story" then
     World:addHandler(6900, 1307, function ()
       if state == 3 then
         state = 4
       end
    end)
  else
     World:error("type passed to CLANNAD:installMainMenuHandler not valid")
  end
end

-- Some paths have Sunohara and other characters being throw around... and will
-- continue until a click breaks them free.
function CLANNAD:clickOnCharactersBeingThrown ()
  -- Different editions of CLANNAD need to have the click injected on different
  -- line numbers.
  name = World:regname()
  if name == "KEY_CLANNAD_ENHD" then
    -- TODO(erg): IMPLEMENT ME.
  else
    World:error("We don't appear to be the English edition of CLANNAD...")
  end
end
