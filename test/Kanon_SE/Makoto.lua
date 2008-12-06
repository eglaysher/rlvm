
World:setRegname("KEY_KANON_SE_ALL")
World:setGameRoot("/home/elliot/GameImages/KANON")

World:import("Kanon_lib.lua")
Kanon:installMainMenuHandler()
Kanon:skipEverything()

-- Makoto's route
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
   "自己紹介を始める",
   "遠慮しておく",
   "思い切り高らかに叫んで驚かしてやる",
   -- January 11th
   "やめる",
   "俺も待ってる",
   "素直に負けを認める",
   "応援だけはしてやる",
   "真琴の部屋にお邪魔する",
   "風呂入って湯冷めしないうちに寝ろと言うんだな",
   "懲りさせてやろう",
   -- January 12th
   "寝るな",
   "商店街に寄って帰る",
   "ついでにタバコも頼む",
   "腹ごしらえのあてを探す",
   "今、この場で追及する",
   "着替えを持って部屋を出た",
   -- January 13th
   "食べてから行く",
   "やっぱり嫌だ",
   "きっぱりと断る",
   "突っ切って帰る",
   "拾って、廊下に投げ放つ",
   -- January 14th
   "好きにさせる",
   "そんな時間ない",
   "家に帰るまで我慢",
   "部屋の模様替えをする",
   -- January 15th
   "絶対にありえない",
   "このままじっとしている",
   "食べることにした",
   -- January 16th
   "作戦その１",
   "隣、歩けよ",
   -- January 17th
   "しばらく眺めている",
   -- January 18th
   "その様子を想像する",
   "猫塚ネコ夫",
   "一緒に入る",
   "寝たままで居る",
   -- January 19th
   "ああ、そうだよ",
-- The walkthrough says this is the next answer, but I don't see it...
--   "そこへ向け、剣を投てきした",
   -- January 20th
   "俺、最近転校してきたんだよ",
   -- January 23rd
   "天野を探していた",
   "大人しく立ち去る",
   -- January 24th
   "もう少しここに居る",
   -- January 25th
   "校門まで戻る",
   -- January 26th
   "天野に違いない"
}

World:setDecisionList(decisions)
