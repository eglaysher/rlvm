state = 0
World:addHandler(9032, 117, function ()
   if state == 0 then
      origin = System:graphics():getChildFgObject(241, 9):getClickPointHack()
      System:event():injectMouseMovement(origin)
      state = 1
   elseif state == 1 then
      System:event():injectMouseDown()
      state = 2
   elseif state == 2 then
      System:event():injectMouseUp()
      state = 3
   elseif state == 4 then
      -- Object 14 is the Exit button
      origin = System:graphics():getChildFgObject(241, 14):getClickPointHack()
      System:event():injectMouseMovement(origin)
      state = 5
   elseif state == 5 then
      System:event():injectMouseDown()
      state = 6
   elseif state == 6 then
      System:event():injectMouseUp()
      state = 7
   end
end)

-- Once we've started a New Game, we shouldn't see the main menu until we are
-- ready to exit.
World:addHandler(628, 27, function ()
  if state == 3 then
     state = 4
  end
end)

-- Based on http://clannad-kouryaku.net/cs_kouryaku.html
decisions = {
    -- 6/28
    "女言葉で喋れ",
    -- 6/30
    "単相直巻整流子電動機",
    "１５５度",
    "ダイヤフラム",
    -- 7/1
    "キス",
    "匂い",
    -- 7/7
    "ともの荷物を確認",
    -- 7/9
    "アパートの前にいた",
    "俺が探す",
    -- 7/10
    "黙って耐えろ",
    -- 7/11
    "所望しない",
    -- 7/14
    "勉強する",
    "一緒に遊ぶ",
    "人は歳とともにアゴがしゃくれていく",
    "さっき神になった",
    "花火の場所に向かう",
    "智代の代わり",
    -- 7/15
    "よくない",
    "勉強する",
    "やらせる",
    -- 7/16
    "勉強する",
    "一緒に作る",
    "俺",
    "智代",
    "鷹文",
    "河南子",
    "選手交代",
    -- 7/20
    "それは避ける",
    "努力するのみ",
    -- 7/23
    "表に出る",
    -- 8/10
    "智代・河南子・俺の三人で行く",
    -- 8/11
    "つっこんでやる",
    -- 8/12
    "学校を作る",
    -- 8/14
    "拒否する"
}

World:setDecisionList(decisions)
