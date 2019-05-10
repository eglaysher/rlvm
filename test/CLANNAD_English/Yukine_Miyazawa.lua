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
  "\"Let Sunohara do it\"",
  "\"Go sleep\"",
  "\"Talk to her a bit more\"",
  "\"Just sleep\"",
   -- April 19th
  "\"Go to the old school building\"",
  "\"To the reference room\"",
   -- April 20th
  "\"Follow him\"",
  "\"See what happens for a bit longer\"",
  "\"Refuse\"", -- Walkthrough backwards?
   -- April 21st
  "\"Wake up like a good student\"",
   -- April 22nd
  "\"Start by introducing yourself\"",
  "\"Go with Sunohara\"",
  "\"Ryou\"",
   -- April 23rd
  "\"Act like an astronaut about to sacrifice himself to save Earth\"", -- Walkthrough doesn't know?
  "\"Don't try the spell\"",
  "\"Don't talk to her\"",
   -- April 24th
  "\"Don't do it\"",
   -- Scenario
  "\"Let her sleep\"",
  "\"It's all right\"",
  "\"Let her sleep\"",
}

World:setDecisionList(decisions)
