World:import("CLANNAD_lib.lua")
CLANNAD:installMainMenuHandler("After Story")
CLANNAD:clickOnCharactersBeingThrown()

-- Custom handler to make sure we can complete the true end to the CLANNAD
-- After Story.
World:addHandler(9031, 39, function ()
  num_lights = Machine:getInt('G', 0)
  if num_lights ~= 13 then
     paths = CLANNAD:routesNeededFor(TrueEndLights)
     routes = table.concat(paths, ", ")
     World:error("There are only " .. num_lights .. " lights! Still need: " .. routes)
  end
end)

-- After Story (
decisions = {
   -- May 5th
   "自分を落ち着かせる",
   -- May 6th
   "いい",
   -- May 9th
   "誤解を解く",
   "黙ってじっとしている",
   -- May 12th
   "テレビを見る",
   -- May 13th
   "創立者祭",
   "模試",
   -- May 16th
   "別にいいけど",
   -- 
   "ダメじゃなくて無理",
   "好き",
   "正直に答える",
   "歌う",
   "さらに続きを訊く",
   "明後日まで待って、一緒にいく",
   "………。",
   "堪える",
   "大人しくテレビを見ている",
   "いらない",
   "そろそろ止める",
   "協力する",
   "見に行く"
}

World:setDecisionList(decisions)
