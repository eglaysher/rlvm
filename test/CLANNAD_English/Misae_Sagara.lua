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
  "\"Go down to the courtyard\"",
  "\"Keep talking\"",
  "\"Watch how things turn out\"",
  "\"I'm not interested in this\"",
  "\"I feel concerned\"",
  "\"Talk to her\"",
  -- April 16th
  "\"Don't skip\"",
  "\"No reason to make that much of an effort\"",
  -- April 17th
  "\"Waste time outside school\"",
  "\"Talk to her\"",
  -- April 18th
  "\"Have her read my fortune\"",
  "\"Return to the classroom after all\"",
  "\"Talk to her\"",
  -- April 19th
  "\"Go straight home\"",
  -- April 20th
  "\"Ignore him\"",
  "\"Ask whether I can come again\"",
  -- April 21st
  "\"Wake up like a good student\"",
  "\"How you scold\"",
  "\"How you worry\"",
  "\"How your futon smells nice\""
}

World:setDecisionList(decisions)
