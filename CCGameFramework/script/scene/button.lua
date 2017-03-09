local Scene = require('script.lib.core.scene')
local Gradient = require('script.lib.ui.gradient')
local LinearLayout = require('script.lib.ui.layout.linear')
local TableLayout = require('script.lib.ui.layout.table')
local Empty = require('script.lib.ui.empty')
local Block = require('script.lib.ui.block')
local Text = require('script.lib.ui.text')
local Button = require('script.lib.ui.comctl.button')
local Edit = require('script.lib.ui.comctl.edit')

local modname = 'ButtonComctlScene'
local M = Scene:new()
_G[modname] = M
package.loaded[modname] = M

function M:new(o)
	o = o or {}
	o.name = 'Button Common Control Scene'
	o.state = {focused=nil, hover=nil}
	setmetatable(o, self)
	self.__index = self
	return o
end

function M:init()
	self.minw = 800
	self.minh = 600
	UIExt.set_minw(self.minw, self.minh)

	UIExt.trace('Scene [Button page] init')
	-- INFO
	local info = UIExt.info()
	-- BG
	local bg = LinearLayout:new({
		right = info.width,
		bottom = info.height
	})
	self.layers.bg = self:add(bg)
	bg:add(Block:new({
		color = '#9C9F3C',
		right = info.width,
		bottom = info.height
	}))
	UIExt.trace('Scene [Button page]: create background #' .. self.layers.bg.handle)
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
	UIExt.trace('Scene [Button page]: create background2 #' .. self.layers.bg2.handle)
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
				FlipScene('ComCtl')
			end
		end
	})
	self.layers.cc = self:add(cc)
	UIExt.trace('Scene [Button page]: create text #' .. self.layers.cc.handle)
	-- TEXT
	local text = Text:new({
		color = '#EEEEEE',
		text = '¡¾°´Å¥¡¿',
		pre_resize = function(this, left, top, right, bottom)
			return left, top, right, bottom / 2
		end
	})
	self.layers.text = self:add(text)
	UIExt.trace('Scene [Button page]: create text #' .. self.layers.text.handle)
	-- MENU
	self:init_menu(info)

	-- EVENT
	self:init_event()

	self.resize(self)
	UIExt.paint()
end

function M:destroy()
	UIExt.trace('Scene [Button page] destroy')
	UIExt.clear_scene()
end

function M:init_event()
	self.handler[self.win_event.created] = function(this)
		UIExt.trace('Scene [Button page] Test created message!')
	end
	self.handler[self.win_event.timer] = function(this, id)
	end
end

function M:init_menu(info)
	-- MENU CONTAINER LAYOUT
	local row = 3
	local col = 5
	local menu = TableLayout:new({
		row = row,
		col = col,
		pre_resize = function(this, left, top, right, bottom)
			local h = top + (bottom - top) / 2
			return left, h, right, h + 250
		end
	})
	self.layers.menu = self:add(menu)
	
	-- MENU BUTTON
	for i=1,row do
		for j=1,col do
			Button:new({
				text = ''..i..'x'..j,
				track_display = 0,
				click = function()
				end
			}):attach(menu)
		end
	end
	-- ALLOCATE BUTTON GROUP
	menu.children[1]:reset('2048')
	menu.children[1].click = function()
		FlipScene('Game_2048')
	end
	menu.children[1].layers.fg:update()

	menu.children[2]:reset('hitokoto')
	menu.children[2].click = function()
		FlipScene('Hitokoto')
	end
	menu.children[2].layers.fg:update()

	menu.children[3]:reset('path')
	menu.children[3].click = function()
		FlipScene('Path')
	end
	menu.children[3].layers.fg:update()

	menu.children[4]:reset('wireworld')
	menu.children[4].click = function()
		FlipScene('WireWorld')
	end
	menu.children[4].layers.fg:update()

	menu.children[5]:reset('music')
	menu.children[5].click = function()
		FlipScene('Music')
	end
	menu.children[5].layers.fg:update()

	UIExt.paint()
end

return M