-- TODO(erg): Add menu navigation code so that this is completely automated.

-- This is based on http://seiya-saiga.com/game/key/kud_wafter.html, along with
-- some random decision making since I only have the All Ages edition, and I
-- assume some decisions were changed for it.
decisions = {
    -- July 19th
    "クラスメイトの男子",
    "知らない２年の女子",
    "食堂のおばさん",
    "魚捕り",
    -- July 21st
    "金物屋",
    "図書館",
    "文房具店",
    "コンビニ",
    "１００円ショップ",
    -- July 22nd
    "そうだタンスだ",
    "はい、そうです",
    "開けてみる",
    "手でお願いする",
    -- July 24th
    "僕の恋人",
    "開けてみる",
    -- July 25th
    "強気なクド",
    "悪戯好きなクド",
    "大人しいクド",
    -- July 26th
    "開けてみる",
    "短パンが気になって",
    -- July 27th
    "ＹＥＳ",
    -- Aug 7th
    "クド",
    -- way to the mile high
    "ぐふー",
    -- Icarus
    "クドに嘘をつかない"
}

World:setDecisionList(decisions)
