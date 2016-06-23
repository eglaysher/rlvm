World:import("CLANNAD_lib.lua")
CLANNAD:installMainMenuHandler("New Game")
CLANNAD:clickOnCharactersBeingThrown()

decisions = {
   -- April 14th
  "\"Just leave it there\"",
   -- April 15th
  "\"Ignore her\"",
  "\"Sit through class\"",
  "\"Go down to the courtyard\"",
  "\"Keep talking\"",
  "\"Escape\"",
  "\"I feel concerned\"",
  "\"Do nothing\"",
  -- April 16th
  "\"Don't skip\"",  -- ?? Ambigious walkthrough
  "\"Run to buy a sandwich\"",
  "\"Help\"",
  "\"I couldn't possibly\"",
  -- April 17th
  "\"Sex appeal\"",  -- ?? Ambigious walkthrough
  "\"Go look for her\"",
  "\"Look in the class next door\"",
  "\"Try talking to her\"",
  -- April 18th
  "\"Let Sunohara do it\"",
  "\"Gotta catch 'er\"",
  "\"An empty classroom\"",
  "\"I'll watch her just a bit more\"",
  "\"Return to the classroom after all\"",
  "\"What kind of guys are you into?\"",
  "\"Grab Furukawa's hand and run past him\"",
  -- April 19th
  "\"Go check on her\"",
  -- April 20th
  "\"What's up with Sanae-san?\"",
  "\"Ignore him\"",
   -- April 21st
  "\"Look out the window\"",
  "\"I'm bored; I should try to make her drink juice through her nose\"",
  "\"Make Furukawa go again\"",
  "\"Make her try again\"",
  -- April 22nd
  "\"Slash\"",
  "\"Ask Fuko for help\"",
  "\"Talk to the Fujibayashi sisters\"",
  "\"Time for a prank\"",
  "Put her around somewhere                 Lv1",
  "\"Take it home\"",
  "\"Don't help\"",
  -- April 23rd
  "\"No\"",
  "\"Tell me you love me 24/7\"",
  "\"No\"",
  "\"Go for a walk\"", -- ??
  "\"Call out to it\"", -- ??
  "\"Time for a prank\"",
  "Put her around somewhere                 Lv2",
  "\"Don't talk to her\"",
  -- April 24th
  "\"Call out to it\"",
  "\"Time for a prank\"",
  "Switch the person she's talking to       Lv1",
  "\"Stay like this\"",
  -- April 25th
  "\"Call out to it\"",
  "Switch the person she's talking to       Lv2",
  -- April 26th
  "\"Ask them what's wrong\"",
  "\"Just leave her here\"",
  "\"Help her\"",
  "Make her drink juice with her nose       Lv2",
  -- April 27th
  "\"Tell her the truth\"",
  "\"Slowly pinch her nose\"",
  "\"I guess so...\"",
  -- April 28th
  "\"Carving of a bear with a salmon in its mouth\"",
  "\"Make her join in the conversation\"",
  "\"Time for a prank\"",
  "Switch the carving she's holding         Lv1",
  -- April 29th
  "\"Follow Nagisa\"",
  "\"Time for a prank\"",
  "Switch the carving she's holding         Lv2",
  -- April 30th
  "\"Reference room\"",  -- ??
  "\"Massage her nose\"",
  -- May 2nd
  "\"Try another teacher\"",
  -- May 3rd
  "\"Rock\"",
  "\"Be THE man\"",
  "\"Can I kiss you\"",
}


World:setDecisionList(decisions)
