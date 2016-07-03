World:import("CLANNAD_lib.lua")
CLANNAD:installMainMenuHandler("New Game")

-- Nagisa's end route
decisions = {
   -- April 14th
  "\"Just leave it there\"",
   -- April 15th
  "\"Apologize\"",
  "\"Sit through class\"",
  "\"Go down to the courtyard\"",
  "\"Keep talking\"",
  "\"Watch how things turn out\"",
  "\"I'm interested in this\"",
  "\"I feel concerned\"",
  "\"Talk to her\"",
  -- April 16th
  "\"Talk to the old lady\"",
  "\"Don't worry about it\"",
  "\"That girl from yesterday...\"",
  "\"Go with Sunohara\"",
  "\"Don't skip\"",
  "\"Help\"",
  "\"I would\"",
  -- April 17th
  "\"Go and watch\"",
  "\"Follow him\"",
  "\"Follow him\"",
  "\"A drawing\"",
  "\"Go look for her\"",
  "\"Look in the class next door\"",
  "\"Try talking to her\"",
  -- April 18th
  "\"Accept\"",
  "\"Kick him back\"",
  "\"She uses playing cards\"",
  "\"Gotta catch 'er\"",
  "\"An empty classroom\"",
  "\"I'll watch her just a bit more\"",
  "\"Return to the classroom after all\"",
  "\"Help her\"",
  "\"What's that drawing on the fliers?\"",
  "\"Grab Furukawa's hand and run past him\"",
   -- April 19th
  "\"Have him stop it already\"",
  "\"Go check on her\"",
  -- April 20th
  "\"How's Furukawa doing?\"",
  "\"Follow him\"",
  "\"See what happens for a bit longer\"",
  "\"Help\"",
  -- April 21st
  "\"Look out the window\"",
  "\"Keep waiting\"",
  "\"Go to negotiate in her place\"",
  -- April 22nd
  "\"Talk to Tomoyo\"",
  "\"Magic\"",
  "\"Ask Fuko for help\"",
  "\"Talk to the Fujibayashi sisters\"",
  "\"Keep waiting\"",
  "\"Take it home\"",
  "\"Don't help\"",
  -- April 23rd
  "\"No\"",
  "\"There is nothing unnecessary, right?\"",
  "\"No\"",
  "\"Sleep\"",
  "\"Talk to her in Sunohara's place\"",
  -- April 24th
  "\"Embrace her\"",
  -- April 26th
  "\"Rush ahead\"",
  "\"Wait until Monday\"",
  "\"Sure\"",
  -- April 27th
  "\"It's all right\"",
  -- April 28th
  "\"Search the new school building\"",
  "\"Look around the second floor\"",
  "\"She'll work\"",  -- "You can"?
  "\"One last chance to have stupid fun\"",
  "\"Stop her\"",
  -- April 30th
  "\"I'll take the last shot\"",
  -- May 1st
  "\"Giving her a present\"",
  -- May 2nd
  "\"I like her\"",
  -- May 10th
  "\"Steal her script\"",
  -- May 11th
  "\"You're a good father\"",
  "\"Yell\"",
}

World:setDecisionList(decisions)
