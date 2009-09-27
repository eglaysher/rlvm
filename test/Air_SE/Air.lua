World:import("Air_lib.lua")
Air:installMainMenuHandler("AIR")

-- Air.
decisions = {
    -- July 16
    "女の子を追う",
    -- July 17
    "戦う",
    -- July 19
    "みすずについてゆく",
    "中に入ってみる",
    "外にでる",
    "戦う",
    -- July 20
    "散歩に出る",
    "せまい道に入ってみる",
    "戦う",
    "迂回する",
    "確かめにいく",
    -- July 21
    "男についてゆく",
    "紙切れを奪う",
    -- July 22
    "もう一度遠い日のことを考える",
    -- July 23
    "ねぐらの中をさまよう",
    "右にいく",
    "右にいく",
}

World:setDecisionList(decisions)
