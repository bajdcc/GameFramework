local Scene = require('script.lib.core.scene')
local Gradient = require('script.lib.ui.gradient')
local AbsoluteLayout = require('script.lib.ui.layout.abs')
local LinearLayout = require('script.lib.ui.layout.linear')
local Empty = require('script.lib.ui.empty')
local Block = require('script.lib.ui.block')
local Text = require('script.lib.ui.text')
local Button = require('script.lib.ui.comctl.button')
local Edit = require('script.lib.ui.comctl.edit')

local modname = 'EditComctlScene'
local M = Scene:new()
_G[modname] = M
package.loaded[modname] = M

function M:new(o)
	o = o or {}
	o.name = 'Edit Common Control Scene'
	o.state = {focused=nil, hover=nil}
	setmetatable(o, self)
	self.__index = self
	return o;
end

function M:init()
	self.minw = 800
	self.minh = 600
	UIExt.set_minw(self.minw, self.minh)

	UIExt.trace('Scene [Edit page] init')
	-- INFO
	local info = UIExt.info()
	-- BG
	local bg = AbsoluteLayout:new({
		right = info.width,
		bottom = info.height
	})
	self.layers.bg = self:add(bg)
	bg:add(Block:new({
		color = '#9C9F3C',
		right = info.width,
		bottom = info.height
	}))
	UIExt.trace('Scene [Edit page]: create background #' .. self.layers.bg.handle)
	-- BG2
	local bg2 = Gradient:new({
		color1 = '#9C9F3C',
		color2 = '#65662A',
		direction = 1,
		pre_resize = function(this, left, top, right, bottom)
			return left, ((bottom - top) / 2) - 100, right, bottom
		end
	})
	self.layers.bg2 = bg:add(bg2)
	UIExt.trace('Scene [Edit page]: create background2 #' .. self.layers.bg2.handle)
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
	UIExt.trace('Scene [Edit page]: create text #' .. self.layers.cc.handle)
	-- TEXT
	local text = Text:new({
		color = '#EEEEEE',
		text = '¡¾ÎÄ±¾¿ò¡¿',
		pre_resize = function(this, left, top, right, bottom)
			return left, top, right, bottom / 2
		end
	})
	self.layers.text = self:add(text)
	UIExt.trace('Scene [Edit page]: create text #' .. self.layers.text.handle)
	-- MENU
	self:init_menu(info)

	-- EVENT
	self:init_event()

	self.resize(self)
	UIExt.paint()
end

function M:destroy()
	UIExt.trace('Scene [Edit page] destroy')
	UIExt.clear_scene()
end

function M:init_event()
	self.handler[self.win_event.created] = function(this)
		UIExt.trace('Scene [Edit page] Test created message!')
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
	Edit:new({
		text = 'Text',
		char_return = function (text)
			CurrentScene.layers.text.text = text
			CurrentScene.layers.text:update_and_paint()
		end,
		char_input = function (text)
			CurrentScene.layers.text.text = text
			CurrentScene.layers.text:update_and_paint()
		end
	}):attach(menu)

	Empty:new():attach(menu)

	Button:new({
		text = '·µ»Ø',
		click = function()
			FlipScene('ComCtl')
		end
	}):attach(menu)
end

return M