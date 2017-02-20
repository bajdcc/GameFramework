local Scene = require('script.lib.core.scene')
local Gradient = require('script.lib.ui.gradient')
local AbsoluteLayout = require('script.lib.ui.layout.abs')
local LinearLayout = require('script.lib.ui.layout.linear')
local Block = require('script.lib.ui.block')
local Text = require('script.lib.ui.text')
local Button = require('script.lib.ui.comctl.button')

local modname = 'CommonControlScene'
local M = Scene:new()
_G[modname] = M
package.loaded[modname] = M

function M:new(o)
	o = o or {}
	o.name = 'Common Control Scene'
	o.state = {focused=nil, hover=nil}
	setmetatable(o, self)
	self.__index = self
	return o;
end

function M:init()
	self.minw = 800
	self.minh = 600
	UIExt.set_minw(self.minw, self.minh)

	UIExt.trace('Scene [ComCtl page] init')
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
	UIExt.trace('Scene [ComCtl page]: create background #' .. self.layers.bg.handle)
	-- BG2
	local bg2 = Gradient:new({
		color1 = '#111111',
		color2 = '#AAAAAA',
		direction = 1,
		pre_resize = function(this, left, top, right, bottom)
			return left, ((bottom - top) / 2) - 100, right, bottom
		end
	})
	self.layers.bg2 = bg:add(bg2)
	UIExt.trace('Scene [ComCtl page]: create background2 #' .. self.layers.bg2.handle)
	-- TEXT
	local cc = Text:new({
		color = '#EEEEEE',
		text = 'Made by bajdcc',
		size = 24,
		pre_resize = function(this, left, top, right, bottom)
			return right - 200, bottom - 50, right, bottom
		end,
		hit = function(this, evt)
			if evt == WinEvent.leftbuttondown then
				FlipScene('Welcome')
			end
		end
	})
	self.layers.cc = self:add(cc)
	UIExt.trace('Scene [Time page]: create text #' .. self.layers.cc.handle)
	-- TEXT
	local text = Text:new({
		color = '#EEEEEE',
		text = '【通用控件】',
		pre_resize = function(this, left, top, right, bottom)
			return left, top, right, bottom / 2
		end
	})
	self.layers.text = self:add(text)
	UIExt.trace('Scene [ComCtl page]: create text #' .. self.layers.text.handle)
	-- MENU
	self:init_menu(info)

	-- EVENT
	self:init_event()

	self.resize(self)
	UIExt.paint()
end

function M:destroy()
	UIExt.trace('Scene [ComCtl page] destroy')
	UIExt.clear_scene()
end

function M:init_event()
	self.handler[self.win_event.created] = function(this)
		UIExt.trace('Scene [ComCtl page] Test created message!')
	end
	self.handler[self.win_event.timer] = function(this, id)
	end
end

function M:init_menu(info)
	-- MENU CONTAINER LAYOUT
	local menu = LinearLayout:new({
		align = 2,
		pre_resize = function(this, left, top, right, bottom)
			local w = left + (right - left) / 2
			local h = top + (bottom - top) / 2
			return w - 100, h, w + 100, h + 250
		end
	})
	self.layers.menu = self:add(menu)
	
	-- MENU BUTTON
	Button:new({
		text = '按钮',
		click = function()
		end
	}):attach(menu)

	Button:new({
		text = '文本',
		click = function()
			FlipScene('Edit')
		end
	}):attach(menu)

	Button:new({
		text = '返回',
		click = function()
			FlipScene('Welcome')
		end
	}):attach(menu)
end

return M