
World:setRegname("KEY_CLANNAD")
World:setGameRoot("/home/elliot/GameImages/CLANNAD/")

World:import("CLANNAD_lib.lua")
CLANNAD:installMainMenuHandler("New Game")
CLANNAD:clickOnCharactersBeingThrown()

decisions = {
   -- April 14th
   "やめておく",
   -- April 15th
   "謝っておく",
   "受ける",
   "中庭に下りる",
   "まだ話を続ける",
   "成り行きを見守る",
   "興味がある",
   "気になる",
   "話しかける",
   -- April 16th
   "婆さんに声をかける",
   "追求してみる",
   "昨日の女の子のこと…",
   "行く",
   "やめておく",
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
   "おっぱい占い",
   "捕まえる",
   "空き教室",
   "もう少し見ている",
   "寝にいく",
   "もう少し話をする",
   "寝ていく",
   "助ける",
   "話しかける",
   -- April 19th
   "もう、やめさせる",
   "旧校舎へ",
   "資料室へ",
   -- April 20th
   "ついていく",
   "もう少し様子を見ている",
   "手伝う",
   -- April 21st
   "真面目に起きていく",
   "声をかける",
   "待ち続ける",
   "そうだな",
   "辛抱する",
   -- April 22nd
   "今、帰るところ",
   "迷惑だ",
   "杏に向かってボタンを投げる",
   "待ち続ける",
   "持って帰る",
   -- April 23rd
   "夕べ眠れずに泣いていたんだろ？",
   "こんにちは、と言え",
   "やらない",
   "気分転換",
   "呼び止める",
   "代わりに出る",
   -- April 24th
   "常に語尾に『それと便座カバー』と付けろ",
   "引き受ける",
   "藤林姉妹",
   "姉",
--   "
   -- Something weird here.
   -- April 29th
   "遠くから見るぐらいいいだろ？"
}

World:setDecisionList(decisions)
