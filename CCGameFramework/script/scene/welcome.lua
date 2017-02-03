local Scene = require('script.lib.core.scene')
local Gradient = require('script.lib.ui.gradient')
local Block = require('script.lib.ui.block')
local Text = require('script.lib.ui.text')

local modname = 'WelcomeScene'
local M = Scene:new()
_G[modname] = M
package.loaded[modname] = M

function M:init()
	UIExt.trace('Scene [Welcome page] init')
	-- INFO
	local info = UIExt.info()
	-- BG
	local bg = Block:new()
	bg.color = '#111111'
	bg.bottom = (info.height / 2) + 100
	self.layers.bg = M:add(bg)
	UIExt.trace('Scene [Welcome page]: create background #' .. self.layers.bg)
	-- BG2
	local bg2 = Gradient:new()
	bg2.color1 = '#111111'
	bg2.color2 = '#AAAAAA'
	bg2.direction = 1
	bg2.top = info.height - bg.bottom
	self.layers.bg2 = M:add(bg2)
	UIExt.trace('Scene [Welcome page]: create background2 #' .. self.layers.bg2)
	-- TEXT
	local text = Text:new();
	text.color = '#EEEEEE'
	text.text = 'Hello world!'
	self.layers.text = M:add(text)
	UIExt.trace('Scene [Welcome page]: create text #' .. self.layers.text)
end

function M:destroy()
	UIExt.trace('Scene [Welcome page] destroy')
	UIExt.clear_scene()
end

return M