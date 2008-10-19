
World:setRegname("KEY_KANON_SE_ALL")
World:setGameRoot("/home/elliot/GameImages/KANON")

World:import("Kanon_lib.lua")
Kanon:installMainMenuHandler()
Kanon:skipLastRegrets()
Kanon:ignoreBlackOnWhiteText()
Kanon:clickAfterEndingCredits()

-- Mai's route
decisions = {
   -- January 7th
   "もう起きる",
   "街を案内してくれないか？",
   "持ってやらない",
   "言い訳する",
   "これ以上は詮索しない",
   "この場所に留まる",
   "遠慮しておく",
   -- January 8th
   "放っておく",
   "その必要はない",
   "いらない",
   "やめておく",
   -- January 9th
   "とにかく急がせる",
   "切り上げさせる",
   "いつからここにいたんだ？",
   "かわす",
   -- January 10th
   "記憶喪失の真偽を確かめる",
   "遠慮しておく",
   "思い切り高らかに叫んで驚かしてやる",
   -- January 11th
   "やめる",
   "俺も待ってる",
   "素直に負けを認める",
   "応援だけはしてやる",
   "ぼ～っとして過ごす",
   "学校にいってノートをとってこいと言うんだな",
   "懲りさせてやろう",
   -- January 12th
   "寝るな",
   "魔物ってなに？",
   "一緒に昼食をとる",
   "試してみる",
   "まっすぐ帰る",
   "我慢する",
   "秋子さんに後は委ねる",
   "夜の校舎に赴く",
   "舞が気づいてくれるまで待つ",
   "渋々体を起こす",
   -- January 13th
   "食べてから行く",
   "やっぱり嫌だ",
   "きっぱりと断る",
   "突っ切って帰る",
   "拾って、廊下に投げ放つ",
   -- January 14th
   "好きにさせる",
   "そんな時間ない",
   "弁護に向かう",
   "家に帰るまで我慢",
   "舞と一緒にいる",
   "部屋の模様替えをする",
   -- January 15th
   "絶対にありえない",
   "このままじっとしている",
   "牛丼を持って逃げる",
   "冷蔵庫に戻しておくよう指示した",
   -- January 16th
   "作戦その１",
   "舞を探す",
   "いつもの場所に向かう",
   "胸を触ってみる",
   -- January 17th
   "適当なことを言って話をはぐらかす",
   "やっぱり遠慮しておく",
   "舞をどこかに誘ってみる",
   -- January 18th
   "正直に答える",
   "俺、邪魔してるよな",
   -- January 19th
   "納豆ごはんを捨て、剣を拾いに走る",
   "真上の天井を見上げた", -- Not sure, walkthrough was incomplete.
   -- January 20th
   "断る",
   "佐祐理さんを探そう",
   -- January 21st
   "好きだった",
   -- January 22nd
   "署名を募る",
   "口を挟む",
   -- January 23rd
   "舞の頬を引っ張った",
   "訊くか、そういうこと？",
   -- January 24th
   "舞に代わって打って出る",
   -- January 25th
   "学食で食べる",
   "右",
   -- January 26th
   -- This walkthrough seems to have a lot of errors in it. Check this later!
--   "訊かない",
--   "読む",
   -- January 27th
--   "あらぬ想像を逞しくしてみる",
   "よけることの大切さ",
   "舞に訊く",
   -- January 28th
   "倉田",
   "じっとしていよう",
   "舞に訊いてみる",
   -- January 29th
--   "与える",
   -- January 30th
   "舞の懐に飛び込んだ",
   "よける",
   "走り抜ける",
   "床を転がる",
   "剣の背で受ける"
}

World:setDecisionList(decisions)
