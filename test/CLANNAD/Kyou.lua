-- luaRlvm file which drives the interpreter through Nagisa's route in
-- CLANNAIDS.

World:setRegname("KEY_CLANNAD")
World:setGameRoot("/home/elliot/GameImages/CLANNAD/")

decisions = {
   -- April 14th
   "上書きで何か吹き込む",
   "やめた。アホらし…",
   -- April 15th
   "謝っておく",
   "受ける",
   "中庭に下りる",
   "まだ話を続ける",
   "避難する",
   "気にならない",
   "放っておく",
   -- April 16th
   "婆さんに声をかける",
   "追求してみる",
   "やめておく",
   -- April 17th
   "校外で暇を潰す",
   "見送る",
   -- April 18th
   "トランプ占い",
   "やっぱり教室に戻る",
   -- April 19th
   "真っ直ぐ帰る",
   -- April 20th
   "無視",
   -- April 21st
   "遅刻していく",
   "待ち構える",
   -- April 22nd
   "こんにちは、と言え",
   "とりあえず持っていく",
   -- April 23rd
   "ついていく",
   "いっこくどう風に言え",
   "出ない",
   -- April 24th
   "マジ？　いいのか？",
   -- April 25th
   "素直に校門へ向かう",
   "恋人",
   -- April 26th
   "それでも良かったら…",
   -- April 28th
   "とりあえず抱いて学校まで",
   "長い髪が好き",
   -- May 1st
   "うまかったぞ…",
   -- May 3rd
   "練習をしてみる",
   -- May 4th
   "アメジスト",
   -- May 5th
   "恋人",
   "キスをする",
   -- May 7th
   "飛び出す"
}

World:setDecisionList(decisions)

-- TODO: Handle exiting the game once we're done with it.
state = 0
World:addHandler(9032, 944, function ()
  -- Object 20 is the New Game button
  if state == 0 then
     origin = System:graphics():getFgObject(20):getClickPointHack()
     System:event():injectMouseMovement(origin)
     state = 1
  elseif state == 1 then
     System:event():injectMouseDown()
     state = 2
  elseif state == 2 then
     System:event():injectMouseUp()
     state = 3
  end
end)

