World:import("CLANNAD_lib.lua")
CLANNAD:installMainMenuHandler("After Story")
CLANNAD:clickOnCharactersBeingThrown()

-- After Story (
decisions = {
  -- May 5th
  "\"Calm down\"",
  -- May 6th
  "\"Yes\"",
  -- May 9th
  "\"Let them think whatever they want\"",
  "\"Deny it\"",
  -- May 12th
  "\"Watch TV\"",
  -- May 13th
  "\"Founder's Festival\"",
  "\"Mock exams\"",
  -- May 16th
  "\"It's fine with me\"",
  "\"Oh, fine\"",
  "\"The old man\"",
  "\"Embrace her\"",
  -- (Furukawa Akio's light GET)
}

World:setDecisionList(decisions)
