local Scene = require('script.lib.core.scene')
local Gradient = require('script.lib.ui.gradient')
local AbsoluteLayout = require('script.lib.ui.layout.abs')
local Block = require('script.lib.ui.block')
local Text = require('script.lib.ui.text')

local modname = 'WelcomeScene'
local M = Scene:new()
_G[modname] = M
package.loaded[modname] = M

M.name = 'Welcome Scene'

function M:init()
	UIExt.trace('Scene [Welcome page] init')
	-- INFO
	local info = UIExt.info()
	-- BG
	local bg = AbsoluteLayout:new({
		right = info.width,
		bottom = info.height
	})
	self.layers.bg = self:add(bg)
	bg:add(Block:new({
		color = '#111111',
		right = info.width,
		bottom = info.height
	}))
	UIExt.trace('Scene [Welcome page]: create background #' .. self.layers.bg.handle)
	-- BG2
	local bg2 = Gradient:new({
		color1 = '#111111',
		color2 = '#AAAAAA',
		direction = 1,
		resize = function(this, left, top, right, bottom)
			this.left, this.top, this.right, this.bottom = left, ((bottom - top) / 2) - 100, right, bottom
			UIExt.update(this)
		end
	})
	self.layers.bg2 = bg:add(bg2)
	UIExt.trace('Scene [Welcome page]: create background2 #' .. self.layers.bg2.handle)
	-- TEXT
	local text = Text:new({
		color = '#EEEEEE',
		text = 'Hello world!',
		right = info.width,
		bottom = info.height
	})
	self.layers.text = self:add(text)
	UIExt.trace('Scene [Welcome page]: create text #' .. self.layers.text.handle)

	-- EVENT
	self:init_event()

	-- TIMER
	UIExt.set_timer(1, 3000)

	self.resize(self)
	UIExt.paint()
end

function M:destroy()
	UIExt.trace('Scene [Welcome page] destroy')
	UIExt.clear_scene()
end

function M:init_event()
	self.handler = {
		[self.win_event.created] = function(this)
			UIExt.trace('Scene [Welcome page] Test created message!')
		end,
		[self.win_event.timer] = function(this, id)
			FlipScene('Time')
		end,
		[self.win_event.leftbuttondown] = function(this, x, y, flags, wheel)
		end,
		[self.win_event.char] = function(this, code, flags)
		end,
		[self.win_event.moved] = self.resize
	}
end

return M