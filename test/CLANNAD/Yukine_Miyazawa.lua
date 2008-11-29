
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
   "無視",
   "もう来ない",
   -- April 21st
   "真面目に起きていく",
   "無視",
   "待ち続ける",
   -- April 22nd
   "こんにちは、と言え",
   "行く",
   "杏",
   "杏に向かってボタンを投げる",
   "待ち続ける",
   "どっかに置いておく",
   -- April 23rd
   "夕べ眠れずに泣いていたんだろ？",
   "常に語尾に『それと便座カバー』と付けろ",
   "やる",
   "眠る",
   "出ない",
   -- April 24th
   "する",
   "試してみる",
   "ではなく…",
   "ではなく…",
   "ではなく…",
   "ではなく…",
   -- Scenario
   "そのままにしておく",
   "いい",
   "このまま寝かす"
}

World:setDecisionList(decisions)
