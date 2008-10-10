-- luaRlvm file which drives the interpreter through Nagisa's route in
-- CLANNAIDS.

World:setRegname("KEY_CLANNAD")
World:setGameRoot("/home/elliot/GameImages/CLANNAD/")

World:import("CLANNAD_lib.lua")
CLANNAD:installMainMenuHandler()

-- Nagisa's end route
decisions = {
   -- April 14th
   "やめておく",
   -- April 15th
   "謝っておく",
   "受ける",
   "中庭に下りる",
   "まだ話を続ける",
   "避難する",
   "気になる",
   "放っておく",
   -- April 16th
   "そのまま通り過ぎる",
   "特に気にしない",
   "昨日の女の子のこと…",
   "やめておく",
   "手伝う",
   "そうしたい",
   -- April 17th
   "イラスト",
   "探しに出る",
   "別の場所を探す",
   -- April 18th
   "おっぱい占い",
   "やっぱり教室に戻る",
   "好きな男のタイプは？",
   "古河の手を引いて、走り抜ける",
   -- April 19th
   "様子を見に行く",
   -- April 20th
   "古河の具合は？",
   "ついていく",
   "もう少し様子を見ている",
   "手伝う",
   -- April 21st
   "窓の外を見る",
   "代わりに談判してくる",
   -- April 22nd
   "魔法",
   "藤林姉妹に相談",
   "手伝わない",
   -- April 23rd
   "違う",
   "違う",
   "出ない",
   -- April 24th
   "抱きしめる",
   -- April 26th
   "探す",
   "幸村をメンバーに入れる",
   -- April 27th
   "手伝いだ",
   "我慢する",
   -- April 28th
   "新校舎で探す",
   "三階を探す",
   "説明する",
   "練習する",
   -- April 30th
   "最後は俺に",
   -- May 1st
   "デートに誘う",
   -- May 2nd
   "好き",
   -- May 10th
   "唇を奪う",
   -- May 11th
   "あんたはいい父親だ",
   "叫ぶ",
}

World:setDecisionList(decisions)
