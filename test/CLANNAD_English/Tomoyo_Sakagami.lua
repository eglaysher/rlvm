World:import("CLANNAD_lib.lua")
CLANNAD:installMainMenuHandler("New Game")
CLANNAD:clickOnCharactersBeingThrown()

-- Nagisa's end route
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
  "\"I'm interested in this\"",
  "\"I feel concerned\"",
  "\"Talk to her\"",
  -- April 16th
  "\"Go with Sunohara\"",
  "\"Don't skip\"",
  "\"Run to buy a sandwich\"",
  "\"Make her do it alone\"",
  -- April 17th
  "\"Go and watch\"",
  "\"Follow him\"",
  "\"Follow him\"",
  "\"Waste time inside school\"",
  "\"Sleep\"",
  "\"Try talking to her\"",
  "\"Talk to her\"",
  -- April 18th
  "\"Accept\"",
  "\"Kick him back\"",
  "\"Let Sunohara do it\"",
  "\"Let her go\"",
  "\"An empty classroom\"",
  "\"I'll watch her just a bit more\"",
  "\"Go sleep\"",
  "\"Leave here\"",
  "\"Help her\"",
  "\"Talk to her\"",
  -- April 19th
  "\"Have him stop it already\"",
  "\"Go straight home\"",
  -- April 20th
  "\"Follow him\"",
  "\"Leave\"",
  "\"Don't come again\"",
  -- April 21st
  "\"Wake up like a good student\"",
  "\"Say something\"",
  "\"Keep waiting\"",
  "\"I guess so\"",
  "\"I eat it anyway\"",
  -- April 22nd
  "\"I'm on my way home\"",
  "\"No, it's not like that\"",
  "\"Have him stop it already\"",
  -- April 23rd
  "\"There is nothing unnecessary, right?\"",
  "\"React to finding me\"",
  "\"Sleep\"",
  "\"About the fact that she's trying to reform the drama club\"",
  -- April 24th
  "\"Perhaps I just want to be with Tomoyo\"",
  -- April 25th
  "\"Start getting ready\"",
  -- April 26th
  "\"Let's eat in the school cafeteria\"",
  -- May 2nd
  "\"Why don't you imagine those things too, then\"",
  -- May 3rd
  "\"Run a simulation\"",
  "\"Let's not\"",
  "\"No... Probably not\"",
  -- May 4th
  "\"Leave for real\"",
  -- May 5th
  "\"Think things can't go on like this\"",
  -- May 6th
  "\"Interfere\"",
  "\"It's not all right\"",
  -- May 11th
  "\"Kick him back\"",
  "\"Help\"",
}

World:setDecisionList(decisions)
