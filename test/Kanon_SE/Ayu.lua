
World:setRegname("KEY_KANON_SE_ALL")
World:setGameRoot("/home/elliot/GameImages/KANON")

World:import("Kanon_lib.lua")
Kanon:installMainMenuHandler()
Kanon:skipLastRegrets()
Kanon:ignoreBlackOnWhiteText()
Kanon:clickAfterEndingCredits()

-- Ayu's route
decisions = {
   -- January 7th
   "もう起きる",
   "街を案内してくれないか？",
   "持ってやらない",
   "言い訳しない",
   "これ以上は詮索しない",
   "一緒に逃げる",
   "貰う",
   -- January 8th
   "放っておく",
   "その必要はない",
   "そうだな…",
   -- January 9th
   "とにかく急がせる",
   "切り上げさせる",
   "誰に会いに来たんだ？",
   "かわす",
   -- January 10th
   "記憶喪失の真偽を確かめる",
   "手伝ってもらう",
   "役に立ったぞ",
   "思い切り高らかに叫んで驚かしてやる",
   -- January 11th
   "試しに入ってみる",
   "素直に謝る",
   "探し物につき合ってやる",
   "ぼ～っとして過ごす",
   "風呂入って湯冷めしないうちに寝ろと言うんだな",
   "懲りさせてやろう",
   -- January 12th
   "寝るな",
   "商店街に寄って帰る",
   "ついでにタバコも頼む",
   "手伝ってやる",
   "我慢する",
   "秋子さんに後は委ねる",
   "渋々体を起こす",
   -- January 13th
   "食べてから行く",
   "やっぱり嫌だ",
   "きっぱりと断る",
   "途中で商店街に寄って傘を買って帰る",
   "今日も探すのか？",
   "拾って、廊下に投げ放つ",
   -- January 14th
   "力ずくでも引き留める",
   "仕方ないので食ってやる",
   "家に帰るまで我慢", -- Guess; not in the walkthrough
   "放っておく", -- Guess; not in the walkthrough
   -- January 15th
   "そういうこともあるかもしれない",
   "俺も出かける",
   "商店街に向かう",
   "冷蔵庫に戻しておくよう指示した",
   -- January 16th
   "作戦その１",
   "帰る",
   -- January 17th
   "普通に答える",
   -- January 19th
   "そうだな",
   "放っておく",
   "名雪に加勢する",
   "フォローする",
   "あゆに頼む",
   -- January 20th
   "もっとからかってみる",
   -- January 21st
   "俺が飲んでやる",
   "迷惑だ、と答える",
   -- January 23rd
   "あゆと一緒に戻る",
   -- January 24th
   "そうだ",
   -- January 25th
   "『オレ』",
   -- January 26th
   "カーテンを開ける",
   "キスをする",
   -- January 27th
   "すぐに後を追いかける"
}

World:setDecisionList(decisions)
