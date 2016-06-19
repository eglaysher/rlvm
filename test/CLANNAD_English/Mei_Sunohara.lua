World:import("CLANNAD_lib.lua")
CLANNAD:installMainMenuHandler("New Game")
CLANNAD:clickOnCharactersBeingThrown()

decisions = {
  -- April 14th
  "\"Record your own voice over it\"",
  "\"Just say whatever comes to mind\"",
  -- April 15th
  "\"Ignore her\"",
  "\"Sit through class\"",
  "\"Do nothing\"",
  "\"Escape\"",
  "\"Do nothing\"",
  -- April 16th
  "\"Don't skip\"",
  -- April 17th
  "\"Waste time outside school\"",
  "\"Watch as she leaves\"",
  -- April 18th
  "\"Have her read my fortune\"",
  "\"Return to the classroom after all\"",
  -- April 19th
  "\"Go straight home\"",
   -- April 20th
  "\"Follow him\"",
  "\"See what happens for a bit longer\"",
  "\"Help\"",
   -- April 21st
  "\"Wake up like a good student\"",
  -- April 22nd
  "\"Say hello\"",
  -- April 23rd
  "\"Say it with water in your mouth\"",
  "\"Talk to her in Sunohara's place\"",
   -- April 24th
  "\"Accept your fate\"",
  "\"Go with the Fujibayashi twins\"",
  "\"The older sister\"",  -- Kyou
   -- April 29th
  "\"Just taking a look from a distance shouldn't hurt, right\"",
}

World:setDecisionList(decisions)
