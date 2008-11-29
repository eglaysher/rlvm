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
   "やめておく",
   -- April 22nd
   "今、帰るところ",
   "いや、そういうのじゃなくて…",
   "もう、やめさせる",
   -- April 23rd
   "夕べ眠れずに泣いていたんだろ？",
   "こんにちは、と言え",
   "気分転換",
   "呼び止める",
   "あいつが演劇部を目指していたことを",
   -- April 24th
   "智代と居たいだけなのかもしれない",
   -- April 25th
   "キスにこだわる",
   -- April 26th
   "学食で食べる",
   -- May 2nd
   "なら、その分、今…",
   "エッチなこと",
   -- May 3rd
   "シミュレーションしてみる",
   "シミュレーションしてみる",
   "ああ…たぶん",
   -- May 4th
   "度の過ぎた妄想をする",
   -- May 5th
   "このままじゃいけないと思った",
   -- May 6th
   "口出しする",
   "よくない",
   -- May 11th
   "蹴り返す",
   "手伝う"
}

World:setDecisionList(decisions)
