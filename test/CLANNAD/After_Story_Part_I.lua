
World:setRegname("KEY_CLANNAD")
World:setGameRoot("/home/elliot/GameImages/CLANNAD/")

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
   "ダメじゃなくて無理",
   "好き",
   "正直に答える",
   "歌う",
   "さらに続きを訊く",
   "明後日まで待って、一緒にいく",
   "………。",
   "堪える",
   "大人しくテレビを見ている",
   "いらない",
   "そろそろ止める",
   "協力する",
   "見に行く"
}

World:setDecisionList(decisions)
