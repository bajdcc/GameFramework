UIExt.trace('Loading script...')

CurrentScene = nil

function PassEventToScene(id, ...)
	UIExt.trace('[!] Event: ' .. id)
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
		Time = require("script.scene.time")
	}
}

FlipScene('Welcome')