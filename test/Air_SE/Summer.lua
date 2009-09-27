World:import("Air_lib.lua")
Air:installMainMenuHandler("SUMMER")

-- No decisions in the summer route.
decisions = {
}

World:setDecisionList(decisions)
