-- luaRlvm file which drives the interpreter through Nagisa's route in
-- CLANNAIDS.

World:setRegname("KEY_CLANNAD")
World:setGameRoot("/home/elliot/GameImages/CLANNAD/")

World:import("CLANNAD_lib.lua")
CLANNAD:installMainMenuHandler("New Game")
CLANNAD:clickOnCharactersBeingThrown()

-- Nagisa's end route
decisions = {
   -- April 14th
   "やめておく",
   -- April 15th
   "ムシする",
   "受ける",
   "中庭に下りる",
   "まだ話を続ける",
   "成り行きを見守る",
   "興味がある",
   "気になる",
   "話しかける",
   -- April 16th
   "行く",
   "やめておく",
   "走って買いにいく",
   "ひとりでやらせる",
   -- April 17th
   "見に行く",
   "ついていく",
   "ついていく",
   "校内で暇を潰す",
   "寝ていく",
   "話しかけてみる",
   "話しかける",
   -- April 18th
   "引き受ける",
   "蹴り返す",
   "自分が占ってもらう",
   "捕まえる",
   "空き教室",
   "もう少し見ている",
   "寝にいく",
   "立ち去る",
   "助ける",
   "話しかける",
   -- April 19th
   "もう、やめさせる",
   "真っ直ぐ帰る",
   -- April 20th
   "無視",
   "また来ていいか訊く",
   -- April 21st
   "真面目に起きていく",
   "声をかける",
   "待ち続ける",
   "そうだな",
   "辛抱する",
   "あくまで美佐枝さんの部屋へ",
   "叱ってくれるところ",
   "心配してくれるところ",
   "布団がいい匂いするところ"
}

World:setDecisionList(decisions)
