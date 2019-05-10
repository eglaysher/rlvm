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
  "\"Clear up the misunderstanding\"",
  "\"Stay silent and don't move\"",
  -- May 12th
  "\"Watch TV\"",
  -- May 13th
  "\"Founder's Festival\"",
  "\"Mock exams\"",
  -- May 16th
  "\"It's fine with me\"",
  "\"It's not too much, it's impossible\"",
  "\"I do\"",
  -- (Furukawa Sanae's light GET)
  "\"Answer honestly\"",
  "\"Sing\"",
  "\"Ask if there's more\"",
  "\"Let Nagisa go alone\"",
  "\"......\"",
  "\"Hold it in\"",
  "\"Quietly keep watching TV\"",
  -- (Yoshino Yusuke's light GET)
  "\"I don't need anything\"",
  "\"Keep watching\"",
  "\"I don't care\"",
  "\"Help him\"",
  -- (Okazaki Naoyuki's light GET)
  "\"Go look\"",
  -- (Ibuki Fuko's light GET)
  -- (Ibuki Kouko's light GET)
}

World:setDecisionList(decisions)
