-- luaRlvm file which drives the interpreter through Nagisa's route in
-- CLANNAIDS.

World:setRegname("KEY_CLANNAD")
World:setGameRoot("/home/elliot/GameImages/CLANNAD/")

-- Kotomi's true end
decisions = {
   -- April 14th
   "やめておく",
   -- April 15th
   "ムシする",
   "サボる",
   "食べる",
   "中庭に下りる",
   "まだ話を続ける",
   "避難する",
   "気にならない",
   "放っておく",
   -- April 17th
   "サボる",
   "ついていく",
   "ページを切ってないか調べる",
   "校外で暇を潰す",
   "見送る",
   -- April 18th
   "春原を占わせる",
   "図書室",
   "脅かしてやる",
   "図書室に入る",
   "名前を呼ぶ",
   "放っておけない",
   -- April 19th
   "真っ直ぐ帰る",
   "後を追って書店に入る",
   "家まで送ると申し出る",
   -- April 20th
   "ここにいていいか訊ねる",
   "読む",
   -- April 21st
   "真面目に起きていく",
   "『違う』と答える",
   -- April 23rd
   "職員室に行ってみる",
   -- April 25th
   "もちろん逃げる",
   -- April 29th
   "外出する",
   -- April 30th
   "ことみを助ける",
   -- May 3rd
   "今はまだ違いますが…",
   -- May 4th
   "このままでもいいか…",
   -- May 5th
   "告白する",
   -- May 8th
   "なぜそんなことを訊くんですか？",
   -- May 10th
   "続ける",
   -- May 11th
   "「あきらめるな」と言う",
   "続ける",
}

World:setDecisionList(decisions)

-- TODO: This now needs to be some sort of library function.
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
