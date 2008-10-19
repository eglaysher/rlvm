
World:setRegname("KEY_KANON_SE_ALL")
World:setGameRoot("/home/elliot/GameImages/KANON")

World:import("Kanon_lib.lua")
Kanon:installMainMenuHandler()
Kanon:skipLastRegrets()
Kanon:ignoreBlackOnWhiteText()

-- Nayuki's route
decisions = {
   -- January 7th
   "もう起きる",
   "街を案内してくれないか？",
   "鞄くらいは持ってやる",
   "言い訳しない",
   "これ以上は詮索しない",
   "一緒に逃げる",
   "貰う",
   -- January 8th
   "部屋に入る",
   "もう少し粘ってみる",
   "送ってもらう",
   "そんなことないぞ",
   "考えてもいい",
   "やめておく",
   -- January 9th
   "とにかく急がせる",
   "もう少し待ってやる",
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
   "風呂入って湯冷めしないうちに寝ろと言うんだな",
   "懲りさせてやろう",
   -- January 12th
   "学校に泊まれ",
   "商店街に寄って帰る",
   "たぶん本気だ",
   "ついでにタバコも頼む",
   "我慢する",
   "秋子さんに後は委ねる",
   "渋々体を起こす",
   -- January 13th
   "俺も名雪の後を追いかける",
   "条件次第で手伝ってもいい",
   "弁当を作ってもらう",
   "突っ切って帰る",
   "拾って、廊下に投げ放つ",
   -- January 14th
   "力ずくでも引き留める",
   "そんな時間ない",
   "何か食っていくか",
   "それくらいなら…",
   "部屋の模様替えをする",
   -- January 15th
   "絶対にありえない",
   "このままじっとしている",
   "冷蔵庫に戻しておくよう指示した",
   -- January 16th
   "作戦その２",
   "帰る",
   -- January 17th
   "安心して、立ち去る",
   -- January 19th
   "そうだな",
   "起こしてやる",
   -- January 20th
   "名雪と一緒に帰る",
   -- January 21st
   "『名雪』っていい名前だよな",
   "さっぱり",
   "一緒に勉強する",
   -- January 22nd
   "慰めの言葉をかける",
   "今日も名雪と勉強する",
   -- January 23rd
   "別に知りたくない",
   "名雪の部屋に行く",
   -- January 25th
   "雑誌を頼む",
   -- January 26th
   "ここで食べる",
   "何か買ってやる",
   -- January 28th
   "ドアをノックする"
}

World:setDecisionList(decisions)
