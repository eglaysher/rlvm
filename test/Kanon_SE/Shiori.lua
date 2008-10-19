
World:setRegname("KEY_KANON_SE_ALL")
World:setGameRoot("/home/elliot/GameImages/KANON")

World:import("Kanon_lib.lua")
Kanon:installMainMenuHandler()
Kanon:skipLastRegrets()
Kanon:ignoreBlackOnWhiteText()

-- Shiori's route
decisions = {
   -- January 7th
   "もう起きる",
   "部屋の片づけを手伝ってくれないか？",
   "諦めた方がいい",
   "絶対に嫌だ",
   "持ってやらない",
   "言い訳する",
   "これ以上は詮索しない",
   "この場所に留まる",
   "遠慮しておく",
   -- January 8th
   "放っておく",
   "送ってもらう",
   "確かに…",
   "部活に入るつもりはない",
   "いらない",
   "やめておく",
   -- January 9th
   "とにかく急がせる",
   "切り上げさせる",
   "誰に会いに来たんだ？",
   "かわす",
   -- January 10th
   "記憶喪失の真偽を確かめる",
   "思い切り高らかに叫んで驚かしてやる",
   -- January 11th
   "やめる",
   "俺は先に戻ってる",
   "何をしに来たのか訊ねる",
   "素直に負けを認める",
   "応援だけはしてやる",
   "ぼ～っとして過ごす",
   "風呂入って湯冷めしないうちに寝ろと言うんだな",
   "懲りさせてやろう",
   -- January 12th
   "寝るな",
   "とりあえず放っておく",
   "何か買ってきてやる",
   "まっすぐ帰る",
   "我慢する",
   "秋子さんに後は委ねる",
   "渋々体を起こす",
   -- January 13th
   "食べてから行く",
   "やっぱり嫌だ",
   "栞のことを話す",
   "やっぱり食べる…",
   "香里のことを栞に訊ねる",
   "突っ切って帰る",
   "拾って、廊下に投げ放つ",
   -- January 14th
   "好きにさせる",
   "そんな時間ない",
   "このまま待っている",
   "家に帰るまで我慢",
   "部屋の模様替えをする",
   -- January 15th
   "絶対にありえない",
   "俺も出かける",
   "学校の方へ歩いてみる",
   "冷蔵庫に戻しておくよう指示した",
   -- January 16th
   "作戦その１",
   "栞の知ってる場所にする",
   "約束する",
   -- January 17th
   "適当なことを言って話をはぐらかす",
   "やっぱり遠慮しておく",
   -- January 18th
   "かわりにくすぐってみる",
   -- January 19th
   "雪合戦につき合う",
   "だったら、似顔絵を見たい",
   -- January 20th
   "好きだと思う",
   "謝る",
   -- January 21st
   "いつもと同じ場所に行く",
   -- January 22nd
   "学食には行かない",
   -- January 23rd
   "そうだな…",
   -- January 24th
   "約束する",
   -- January 25th
   "期待してるぞ",
   -- January 26th
   "無茶をしてでも食う",
   -- January 27th
   "恋人だ",
   -- January 29th
   "ふたりを自分のテーブルに呼んだ",
   -- January 30th
   "考えてるかもしれない",
   "栞を求める"
}

World:setDecisionList(decisions)
