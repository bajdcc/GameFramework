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
local WATM = require('script.lib.ui.watm')

local modname = 'PathScene'
local M = Scene:new()
_G[modname] = M
package.loaded[modname] = M

function M:new(o)
	o = o or {}
	o.name = 'Wireworld Scene'
	o.def = {
		timerid = 10
	}
	setmetatable(o, self)
	self.__index = self
	return o
end

function M:init()
	self.minw = 800
	self.minh = 600
	UIExt.set_minw(self.minw, self.minh)

	UIExt.trace('Scene [Wireworld Page] init')
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
	UIExt.trace('Scene [Wireworld Page]: create background #' .. self.layers.bg.handle)
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
				FlipScene('Button')
			end
		end
	})
	self.layers.cc = self:add(cc)
	UIExt.trace('Scene [Wireworld Page]: create text #' .. self.layers.cc.handle)
	-- TEXT
	local text = Text:new({
		color = '#222222',
		text = 'Wireworld Automaton',
		family = 'Times New Roman',
		pre_resize = function(this, left, top, right, bottom)
			return left, top, right, top + 50
		end
	})
	self.layers.text = self:add(text)
	UIExt.trace('Scene [Wireworld Page]: create text #' .. self.layers.text.handle)
	-- MENU
	self:init_menu(info)

	-- EVENT
	self:init_event()

	UIExt.set_timer(8, 500)

	self.resize(self)
	UIExt.paint()
end

function M:destroy()
	UIExt.trace('Scene [Wireworld Page] destroy')
	UIExt.clear_scene()
end

function M:init_event()
	self.handler[self.win_event.created] = function(this)
		UIExt.trace('Scene [Wireworld Page] Test created message!')
	end
	self.handler[self.win_event.timer] = function(this, id)
		if id == 8 then
			this.layers.watm.text = './script/lib/resource/wireworld_computer.txt'
			this.layers.watm:update_and_paint()
			UIExt.kill_timer(8)
			UIExt.set_timer(5, 30)
		elseif id == 5 then
			UIExt.refresh(this.layers.watm)
			UIExt.paint()
		end
	end
	self.handler[self.win_event.keydown] = function(this, code, flags)
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
	self.layers.watm = bg:add(WATM:new())
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
			CurrentScene.def.type = 1
			path_restart(CurrentScene.def)
		end
	}):attach(slider)
	Button:new({
		text = '暂停/继续',
		font_family = '楷体',
		track_display = 0,
		font_size = 16,
		click = function()
			CurrentScene.def.type = 2
			path_restart(CurrentScene.def)
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
end

return M