World:import("CLANNAD_lib.lua")
CLANNAD:installMainMenuHandler("After Story")
CLANNAD:clickOnCharactersBeingThrown()

-- Custom handler to make sure we can complete the true end to the CLANNAD
-- After Story.
World:addHandler(9031, 39, function ()
  num_lights = Machine:getInt('G', 0)
  if num_lights ~= 13 then
     paths = CLANNAD:routesNeededFor(TrueEndLights)
     routes = table.concat(paths, ", ")
     World:error("There are only " .. num_lights .. " lights! Still need: " .. routes)
  end
end)

-- After Story (
decisions = {
  -- May 5th
  "\"Calm down\"",
  -- May 6th
  "\"Yes\"",
  -- May 9th
  "\"Clear up the misunderstanding\"",
  "\"Stay silent and don't move\"",
  -- May 12th
  "\"Watch TV\"",
  -- May 13th
  "\"Founder's Festival\"",
  "\"Mock exams\"",
  -- May 16th
  "\"It's fine with me\"",
  "\"It's not too much, it's impossible\"",
  "\"I do\"",
  "\"Answer honestly\"",
  "\"Sing\"",
  "\"Ask if there's more\"",
  "\"Let Nagisa go alone\"",
  "\"......\"",
  "\"Hold it in\"",
  "\"Quietly keep watching TV\"",
  "\"I don't need anything\"",
  "\"Keep watching\"",
  "\"I don't care\"",
  "\"Help him\"",
  "\"Go look\"",
  "\"Call out\"",
}

World:setDecisionList(decisions)
