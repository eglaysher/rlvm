World:import("CLANNAD_lib.lua")
CLANNAD:installMainMenuHandler("New Game")
CLANNAD:clickOnCharactersBeingThrown()

decisions = {
   -- April 14th
  "\"Record your own voice over it\"",
  "\"Just say whatever comes to mind\"",
   -- April 15th
  "\"Apologize\"",
  "\"Sit through class\"",
  "\"Do nothing\"",
  "\"Escape\"",
  "\"Do nothing\"",
   -- April 16th
  "\"Talk to the old lady\"",
  "\"Try pressing for details\"",
  "\"Don't skip\"",
   -- April 17th
  "\"Waste time outside school\"",
  "\"Watch as she leaves\"",
   -- April 18th
  "\"She uses boobs\"",
  "\"Return to the classroom after all\"",
   -- April 19th
  "\"Go straight home\"",
   -- April 20th
  "\"Follow him\"",
  "\"See what happens for a bit longer\"",
  "\"Help\"",
   -- April 21st
  "\"Be late\"",
  "\"Jump right\"",
  "\"Try shaking the person\"",
  "\"Find the address and give it back\"",
  -- April 22nd
  "\"Start by introducing yourself\"",
  "\"Throw Botan toward Kyou\"",
  "\"Don't help\"",
   -- April 23rd
  "\"Act like we're lovers parting ways at a train station\"",
  "\"Don't talk to her\"",
   -- April 24th
  "\"Don't play\"",
   -- Kappei's Scenario
  "\"Find him a place to stay\"",
}

World:setDecisionList(decisions)
