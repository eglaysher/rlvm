World:import("CLANNAD_lib.lua")
CLANNAD:installMainMenuHandler("After Story")
CLANNAD:clickOnCharactersBeingThrown()

-- After Story (
decisions = {
   -- May 5th
   "自分を落ち着かせる",
   -- May 6th
   "いい",
   -- May 9th
   "誤解を解く",
   "黙ってじっとしている",
   -- May 12th
   "テレビを見る",
   -- May 13th
   "創立者祭",
   "模試",
   -- May 16th
   "別にいいけど",
   -- 
   "仕方がない…",
   "オッサンのこと",
   "おはよう、と返す",
}

World:setDecisionList(decisions)
