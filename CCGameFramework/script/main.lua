UIExt.trace('Loading script...')

CurrentScene = nil

local WelcomeScene = require("script.scene.welcome")
local Welcome = WelcomeScene:new();
Welcome:FlipScene()