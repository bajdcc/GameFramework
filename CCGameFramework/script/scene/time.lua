local Scene = require('script.lib.core.scene')
local Gradient = require('script.lib.ui.gradient')
local Block = require('script.lib.ui.block')
local Text = require('script.lib.ui.text')
local QR = require('script.lib.ui.qr')

local modname = 'TimeScene'
local M = Scene:new()
_G[modname] = M
package.loaded[modname] = M

M.name = 'Time Scene'

function M:init()
	UIExt.trace('Scene [Time page] init')
	-- INFO
	local info = UIExt.info()
	-- VAR
	self.hue = 0
	-- BG
	local bg = Block:new({
		color = UIExt.hsb2rgb(self.hue, 128, 128),
		right = info.width,
		bottom = info.height
	})
	self.layers.bg = self:add(bg)
	UIExt.trace('Scene [Time page]: create background #' .. self.layers.bg.handle)
	-- TEXT
	local text = Text:new({
		color = '#EEEEEE',
		text = 'Time!',
		right = info.width,
		bottom = info.height
	})
	self.layers.text = self:add(text)
	UIExt.trace('Scene [Time page]: create text #' .. self.layers.text.handle)
	-- TEXT
	local cc = Text:new({
		color = '#EEEEEE',
		text = 'Made by bajdcc',
		size = 24,
		resize = function(this, left, top, right, bottom)
			this.left, this.top, this.right, this.bottom = right - 200, bottom - 50, right, bottom
			UIExt.update(this)
		end
	})
	self.layers.cc = self:add(cc)
	UIExt.trace('Scene [Time page]: create text #' .. self.layers.cc.handle)
	-- QR
	local qr = QR:new({
		color = '#111111',
		resize = function(this, left, top, right, bottom)
			this.left, this.top, this.right, this.bottom = right - 170, bottom - 190, right - 30, bottom - 50
			UIExt.update(this)
		end
	})
	self.layers.qr = self:add(qr)
	UIExt.trace('Scene [Time page]: create qr #' .. self.layers.qr.handle)

	-- EVENT
	self:init_event()

	-- TIMER
	UIExt.set_timer(1, 1000)
	UIExt.set_timer(2, 60000)
	UIExt.set_timer(5, 100)

	self.resize(self)
	UIExt.paint()
end

function M:destroy()
	UIExt.trace('Scene [Time page] destroy')
	UIExt.clear_scene()
end

function M:init_event()
	self.handler = {
		[self.win_event.created] = function(this)
			UIExt.trace('Scene [Time page] Test created message!');
		end,
		[self.win_event.timer] = function(this, id)
			if id == 5 then
				if this.hue > 240 then
					this.hue = 0
				end
				this.hue = this.hue + 1
				this.layers.bg.color = UIExt.hsb2rgb(this.hue, 128, 128)
				this.layers.bg:update()
				UIExt.paint()
			elseif id == 1 then
				this.layers.text.text = os.date("%Y-%m-%d %H:%M:%S")
				this.layers.text:update()
			elseif id == 2 then
				FlipScene('Welcome')
			end
		end,
		[self.win_event.leftbuttondown] = function(this, x, y, flags, wheel)
		end,
		[self.win_event.char] = function(this, code, flags)
		end,
		[self.win_event.moved] = self.resize
	}
end

return M