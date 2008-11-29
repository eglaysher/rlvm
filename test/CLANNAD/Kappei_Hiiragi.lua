-- luaRlvm file which drives the interpreter through Nagisa's route in
-- CLANNAIDS.

World:setRegname("KEY_CLANNAD")
World:setGameRoot("/home/elliot/GameImages/CLANNAD/")

World:import("CLANNAD_lib.lua")
CLANNAD:installMainMenuHandler("New Game")
CLANNAD:clickOnCharactersBeingThrown()

decisions = {
   -- April 14th
   "上書きで何か吹き込む",
   "やめた。アホらし…",
   -- April 15th
   "謝っておく",
   "受ける",
   "中庭に下りる",
   "まだ話を続ける",
   "避難する",
   "気にならない",
   "放っておく",
   -- April 16th
   "婆さんに声をかける",
   "追求してみる",
   "やめておく",
   -- April 17th
   "校外で暇を潰す",
   "見送る",
   -- April 18th
   "トランプ占い",
   "やっぱり教室に戻る",
   -- April 19th
   "真っ直ぐ帰る",
   -- April 20th
   "ついていく",
   "もう少し様子を見ている",
   "手伝う",
   -- April 21st
   "遅刻していく",
   "右に跳ぶ",
   "ゆすってみる",
   "住所を調べて届けてやる",
   -- April 22nd
   "俺を見つけるリアクションから入れ",
   "杏に向かってボタンを投げる",
   "手伝わない",
   -- April 23rd
   "ホームで離ればなれになる恋人のように",
   "出ない",
   -- April 24th
   "しない",
   -- Kappei's Scenario
   "宿を紹介してやる"
}

World:setDecisionList(decisions)
