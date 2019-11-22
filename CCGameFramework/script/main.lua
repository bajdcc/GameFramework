UIExt.trace('Loading script...')

require("script.lib.core.winevt")

CurrentScene = nil
CurrentHitTest = HitTest.nodecision
CurrentCursor = SysCursor.arrow
CurrentCursorX = 0
CurrentCursorY = 0

local DebugSkipMsg = {
	[WinEvent.mousemove] = true,
	[WinEvent.mousehover] = true
}

function PassEventToScene(id, ...)
	if DebugSkipMsg[id] == nil then
		UIExt.trace('[' .. CurrentScene.name .. '] Event: ' .. id)
	end
	return CurrentScene:event(id, ...)
end

function FlipScene(scene)
	if CurrentScene then
		CurrentScene:event(CurrentScene.win_event.destroyed)
		CurrentScene:destroy()
	end
	local sence_name = Window.Scene[scene]
	package.loaded[sence_name]  = nil
	CurrentScene = require(Window.Scene[scene]):new()
	CurrentScene:init()
	CurrentScene:event(CurrentScene.win_event.created)
	UIExt.paint()
end

Window = {
	Scene = {
		-- MAIN
		Welcome = "script.scene.welcome",
		Time = "script.scene.time",
		ComCtl = "script.scene.comctl",
		Edit = "script.scene.edit",
		Button = "script.scene.button",
		-- GAME
		Game_2048 = "script.scene.game.2048",
		Mice2d = "script.scene.game.mice",
		-- WEB
		Hitokoto = "script.scene.web.hitokoto",
		Music = "script.scene.web.music",
		-- VISUAL
		Path = "script.scene.visual.path",
		WireWorld = "script.scene.visual.wireworld",
		PE2D = "script.scene.visual.2dpe",
		CG = "script.scene.visual.cg",
		clib2d = "script.scene.visual.clib2d",
		-- X86
		X86 = "script.scene.simu.x86",
		Parser2d = "script.scene.visual.parser2d",
	}
}

FlipScene('Welcome')