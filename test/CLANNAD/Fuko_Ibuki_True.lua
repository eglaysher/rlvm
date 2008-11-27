-- luaRlvm file which drives the interpreter through Nagisa's route in
-- CLANNAIDS.

World:setRegname("KEY_CLANNAD")
World:setGameRoot("/home/elliot/GameImages/CLANNAD/")

World:import("CLANNAD_lib.lua")
CLANNAD:installMainMenuHandler()
CLANNAD:clickOnCharactersBeingThrown()

decisions = {
   -- April 14th
   "やめておく",
   -- April 15th
   "ムシする",
   "受ける",
   "中庭に下りる",
   "まだ話を続ける",
   "避難する",
   "気になる",
   "放っておく",
   -- April 16th
   "やめておく",
   "走って買いにいく",
   "手伝う",
   "そうしたい",
   -- April 17th
   "コラボレーション",
   "探しに出る",
   "隣の教室を覗いてみる",
   "話しかけてみる",
   -- April 18th
   "自分が占ってもらう",
   "捕まえる",
   "空き教室",
   "もう少し見ている",
   "やっぱり教室に戻る",
   "ビラの絵はなんなんですか？",
   "古河の手を引いて、走り抜ける",
   -- April 19th
   "様子を見に行く",
   -- April 20th
   "早苗さんは？",
   "無視",
   -- April 21st
   "窓の外を見る",
   "暇だから、鼻からジュースを飲ませてみる",
   "代わりに談判してくる",
   -- April 22nd
   "斬る",
   "風子に相談してみる",
   "担任に相談",
   "悪戯をする",
   "鼻からジュースを飲ませる　　 Lv2",
   "持って帰る",
   "手伝わない",
   -- April 23rd
   "違う",
   "四六時中も好きと言って",
   "違う",
   "気分転換",
   "呼び止める",
   "悪戯をする",
   "どこかに置いてくる　　　　　 Lv1",
   "出ない",
   -- April 24th
   "呼びとめる",
   "悪戯をする",
   "話している相手をすり替える　 Lv1",
   "そのままでいる",
   -- April 25th
   "呼びとめる",
   "話している相手をすり替える　 Lv2",
   -- April 26th
   "訊いてみる",
   "力ずくで連れていく",
   "手伝うことにした",
   "どこかに置いてくる　　　　　 Lv2",
   -- April 27th
   "正直にそのことを告げる",
   "おもむろに鼻をつまむ",
   "そうだな…",
   -- April 28th
   "鮭をくわえた熊",
   "話し合いに参加させる",
   "悪戯をする",
   "抱いている彫刻をすり替える　 Lv1",
   -- April 29th
   "ここに残る",
   "悪戯をする",
   "抱いている彫刻をすり替える　 Lv2",
   -- April 30th
   "新校舎三階の廊下",
   "鼻を揉む",
   -- May 2nd
   "無理を言う",
   -- May 11th
   "代わる",
   --
   "キスしていいか？"
}


World:setDecisionList(decisions)
