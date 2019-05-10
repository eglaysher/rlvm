World:import("CLANNAD_lib.lua")
CLANNAD:installMainMenuHandler("New Game")
CLANNAD:clickOnCharactersBeingThrown()

decisions = {
   -- April 14th
  "\"Just leave it there\"",
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
  "\"She uses playing cards\"",
  "\"Return to the classroom after all\"",
  -- April 19th
  "\"Go straight home\"",
  -- April 20th
  "\"Ignore him\"",
  -- April 21st
  "\"Be late\"",
  "\"Jump right\"",
  "\"Leave\"",
  "\"Tear it up\"",
  -- April 22nd
  "\"Say hello\"",
  "\"Take him for now\"",
  -- April 23rd
  "\"Follow her\"",
  "\"Always add \\\"and a toilet seat cover\\\" to the end of your sentences\"",
  "\"Don't talk to her\"",
   -- April 24th
  "\"Really? You will?\"",
   -- April 25th
  "\"Just go to the school gate\"",
  "\"Friends\"",
  -- April 26th
  "\"Let's give it a try\"",
  -- April 28th
  "\"Bring him along to school\"",
  "\"Long hair\"",
  -- May 1st
  "\"It's terrible...\"",
  -- May 3rd
  "\"Try practicing\"",
  -- May 4th
  "\"Amethyst\"",
  -- May 5th
  "\"Lovers\"",
  "\"Kiss her\"",
   -- May 7th
  "\"Run out\"",
}

World:setDecisionList(decisions)
