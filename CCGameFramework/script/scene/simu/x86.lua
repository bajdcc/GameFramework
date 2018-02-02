local Scene = require('script.lib.core.scene')
local Gradient = require('script.lib.ui.gradient')
local AbsoluteLayout = require('script.lib.ui.layout.abs')
local TableLayout = require('script.lib.ui.layout.table')
local LinearLayout = require('script.lib.ui.layout.linear')
local Empty = require('script.lib.ui.empty')
local Block = require('script.lib.ui.block')
local Text = require('script.lib.ui.text')
local Button = require('script.lib.ui.comctl.button')
local Edit = require('script.lib.ui.comctl.edit')
local Radius = require('script.lib.ui.radius')
local X86 = require('script.lib.ui.x86')

local modname = 'script.scene.simu.x86'
local M = Scene:new()
_G[modname] = M
package.loaded[modname] = M

function M:new(o)
	o = o or {}
	o.name = 'Simu-X86 Scene'
	o.def = {
		timerid = 10,
		state = true
	}
	setmetatable(o, self)
	self.__index = self
	return o
end

function M:init()
	self.minw = 800
	self.minh = 600
	UIExt.set_minw(self.minw, self.minh)

	UIExt.ui_set_value("X86-TICK", 0)
	self.tick = 0
	UIExt.trace('Scene [Simu-X86 Page] init')
	-- INFO
	local info = UIExt.info()
	-- BG
	local bg = LinearLayout:new({
		right = info.width,
		bottom = info.height
	})
	self.layers.bg = self:add(bg)
	bg:add(Block:new({
		color = '#EEEEEE',
		right = info.width,
		bottom = info.height
	}))
	UIExt.trace('Scene [Simu-X86 Page]: create background #' .. self.layers.bg.handle)
	-- TEXT
	local cc = Text:new({
		color = '#222222',
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
	UIExt.trace('Scene [Simu-X86 Page]: create text #' .. self.layers.cc.handle)
	-- TEXT
	local text = Text:new({
		color = '#222222',
		text = 'X86系列模拟器',
		family = '楷体',
		pre_resize = function(this, left, top, right, bottom)
			return left, top, right, top + 50
		end
	})
	self.layers.text = self:add(text)
	UIExt.trace('Scene [Simu-X86 Page]: create text #' .. self.layers.text.handle)
	-- MENU
	self:init_menu(info)

	-- EVENT
	self:init_event()

	UIExt.ui_set_value("X86-KBD", 0)

	UIExt.set_timer(8, 500)
	UIExt.set_timer(10, 1000)

	self.resize(self)
	UIExt.paint()
end

function M:destroy()
	UIExt.trace('Scene [Simu-X86 Page] destroy')
	UIExt.clear_scene()
end

function M:init_event()
	self.path = ''
	self.handler[self.win_event.created] = function(this)
		UIExt.trace('Scene [Simu-X86 Page] Test created message!')
	end
	self.handler[self.win_event.timer] = function(this, id)
		if id == 8 then
			this.layers.x86.text = this.path
			this.layers.x86:update_and_paint()
			UIExt.kill_timer(8)
			UIExt.set_timer(5, 30)
		elseif id == 5 and this.def.state then
			UIExt.refresh(this.layers.x86, 0)
			UIExt.paint()
		elseif id == 10 and this.def.state then
			UIExt.refresh(this.layers.x86, 10)
			local tick = UIExt.ui_get_value("X86-TICK")
			local span = tick - this.tick
			this.tick = tick
			if span > 1000000 then
				this.layers.rtstatus.text = string.format("IPS: %.2fM", span / 1000000)
			elseif span > 1000 then
				this.layers.rtstatus.text = string.format("IPS: %.2fK", span / 1000)
			else
				this.layers.rtstatus.text = string.format("IPS: %d", span)
			end
			this.layers.rtstatus:update_and_paint()
		elseif id == 12 then
			this.layers.restart_btn:enabled()
			UIExt.paint()
			UIExt.kill_timer(12)
		end
	end
	self.handler[self.win_event.keydown] = function(this, code, scan, flags)
		UIExt.ui_set_value("X86-KBD", scan)
		UIExt.refresh(this.layers.x86, 12)
	end
	self.handler[self.win_event.keyup] = function(this, code, scan, flags)
		UIExt.ui_set_value("X86-KBD", scan | 0x80000000)
		UIExt.refresh(this.layers.x86, 12)
	end
end

function M:init_menu(info)
	local bg = LinearLayout:new({
		padleft = 10,
		padtop = 60,
		padright = 10,
		padbottom = 60
	})
	self:add(bg)
	self.layers.x86 = bg:add(X86:new())
	local menu = LinearLayout:new({
		row = row,
		col = col,
		padleft = 1,
		padtop = 1,
		padright = 1,
		padbottom = 1
	})
	self.layers.menu = bg:add(menu)
	local content = LinearLayout:new({
		padleft = 1,
		padtop = 1,
		padright = 1,
		padbottom = 1,
	})
	content:attach(self.layers.menu)
	local slider = LinearLayout:new({
		align = 1,
		padleft = 2,
		padtop = 2,
		padright = 2,
		padbottom = 2,
		pre_resize = function(this, left, top, right, bottom)
			return left, bottom - 50, left + 350, bottom
		end
	})
	self:add(slider)
	Button:new({
		text = '重新开始',
		font_family = '楷体',
		track_display = 0,
		font_size = 16,
		click = function()
			UIExt.refresh(CurrentScene.layers.x86, 5)
			CurrentScene.layers.restart_btn:disabled()
			UIExt.set_timer(12, 6000)
			UIExt.paint()
		end
	}):attach(slider)
	self.layers.restart_btn = slider.children[#slider.children]
	self.layers.restart_btn:disabled()
	UIExt.set_timer(12, 6000)
	Button:new({
		text = '放缩',
		font_family = '楷体',
		track_display = 0,
		font_size = 16,
		click = function()
			UIExt.refresh(CurrentScene.layers.x86, 14)
		end
	}):attach(slider)
	Text:new({
		text = '状态',
		family = '楷体',
		size = 16,
		click = function()
		end
	}):attach(slider)
	self.layers.rtstatus = slider.children[#slider.children]
	UIExt.paint()
end

return M