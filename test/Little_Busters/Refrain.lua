World:import("LB_lib.lua")
-- TODO: The normal main menu handler isn't enough; we need something that
-- deals with the original main menu and the second one.
-- LB:installMainMenuHandler()

-- TODO: Needs several clicks in the middle. Also needs a click at the end?

-- Refrain (Little Busters)
-- Adapted from http://gamerssquare.kanpaku.jp/littlebusters.htm#r9
decisions = {
    -- May 21st
    "頭を使って",
    -- May 23rd
    "訊く",
    -- May 24th
    "訊いてみる",
    "怪我をしていることを思い出したから",
    -- 修学旅行
    "鈴を連れて逃げる",
    "いや",
    -- Epilogue
    "よくない",
    -- 修学旅行
    "冷静になる",
    "枝を折る",
    "後回しにする",
    "荷物を調べてもらう",
    "何かを考える",
    "恭介を助ける"
}

World:setDecisionList(decisions)
