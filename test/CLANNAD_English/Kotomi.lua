World:import("CLANNAD_lib.lua")
CLANNAD:installMainMenuHandler("New Game")
CLANNAD:clickOnCharactersBeingThrown()

-- Kotomi's true end
decisions = {
  -- April 14th
  "\"Record your own voice over it\"",
  "\"Just say whatever comes to mind\"",
  -- April 15th
  "\"Ignore her\"",
  "\"Skip class\"",
  "\"Eat\"",
  "\"Go down to the courtyard\"",
  "\"Keep talking\"",
  "\"Escape\"",
  "\"I don't feel concerned\"",
  "\"Do nothing\"",
  -- April 17th
  "\"Skip class\"",
  "\"Follow her\"",
  "\"Check for cut-up pages\"",
  "\"Waste time outside school\"",
  "\"Watch as she leaves\"",
  -- April 18th
  "\"Let Sunohara do it\"",
  "\"The library\"",
  "\"Surprise her\"",
  "\"Enter the library\"",
  "\"Call her name\"",
  "\"I can't leave her alone\"",
  -- April 19th
  "\"Go straight home\"",
  "\"Follow her into the bookstore\"",
  "\"Offer to walk her home\"",
  -- April 20th
  "\"Ask if it's all right to stay\"",
  "\"Read\"",
  -- April 21st
  "\"Wake up like a good student\"",
  "\"Deny it\"",
  -- April 23rd
  "\"Go to the teacher's office\"",
  -- April 25th
  "\"Wait for her\"",
   -- April 29th
  "\"Go outside\"",
   -- April 30th
  "\"Help Kotomi\"",
  -- May 3rd
  "\"Not yet...\"",
  -- May 4th
  "\"Just keep things the way they are\"",
  -- May 5th
  "\"Confess\"",
   -- May 8th
  "\"Why ask something like that\"",
   -- May 10th
  "\"Keep going\"",
   -- May 11th
  "\"Tell them they can't give up\"",
  "\"Keep going\"",
}

World:setDecisionList(decisions)
