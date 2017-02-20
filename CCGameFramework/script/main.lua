UIExt.trace('Loading script...')

require("script.lib.core.winevt")

CurrentScene = nil
CurrentHitTest = HitTest.nodecision
CurrentCursor = SysCursor.arrow

local DebugSkipMsg = {
	[WinEvent.mousemove] = true,
	[WinEvent.mousehover] = true
}

function PassEventToScene(id, ...)
	if DebugSkipMsg[id] == nil then
		UIExt.trace('[' .. CurrentScene.name .. '] Event: ' .. id)
	end
	CurrentScene:event(id, ...)
end

function FlipScene(scene)
	if CurrentScene then
		CurrentScene:event(CurrentScene.win_event.destroyed)
		CurrentScene:destroy()
	end
	CurrentScene = Window.Scene[scene]:new()
	CurrentScene:init()
	CurrentScene:event(CurrentScene.win_event.created)
	UIExt.paint()
end

Window = {
	Scene = {
		Welcome = require("script.scene.welcome"),
		Time = require("script.scene.time"),
		ComCtl = require("script.scene.comctl"),
		Edit = require("script.scene.edit")
	}
}

FlipScene('Welcome')