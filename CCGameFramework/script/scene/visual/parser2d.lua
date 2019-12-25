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
local PE2D = require('script.lib.ui.parser2d')

local modname = 'script.scene.visual.parser2d'
local M = Scene:new()
_G[modname] = M
package.loaded[modname] = M

function M:new(o)
	o = o or {}
	o.name = '脚本操作系统'
	o.def = {
		timerid = 10,
		state = true
	}
	setmetatable(o, self)
	self.__index = self
	return o
end

function M:init()
	self.minw = 900
	self.minh = 600
	UIExt.set_minw(self.minw, self.minh)

	UIExt.trace('Scene [Parser 2D Engine] init')
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
	UIExt.trace('Scene [Parser 2D Engine]: create background #' .. self.layers.bg.handle)
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
	UIExt.trace('Scene [Parser 2D Engine]: create text #' .. self.layers.cc.handle)
	-- TEXT
	local text = Text:new({
		color = '#222222',
		text = self.name,
		family = '楷体',
		pre_resize = function(this, left, top, right, bottom)
			return left, top, right, top + 50
		end
	})
	self.layers.text = self:add(text)
	UIExt.trace('Scene [Parser 2D Engine]: create text #' .. self.layers.text.handle)
	-- MENU
	self:init_menu(info)

	-- EVENT
	self:init_event()

	UIExt.set_timer(8, 500)

	self.resize(self)
	UIExt.paint()
end

function M:destroy()
	UIExt.trace('Scene [Parser 2D Engine] destroy')
	UIExt.clear_scene()
end

function M:init_event()
	self.handler[self.win_event.created] = function(this)
		UIExt.trace('Scene [Parser 2D Engine] Test created message!')
	end
	self.handler[self.win_event.char] = function(this, code, scan, flags)
		UIExt.refresh(CurrentScene.layers.pe2d, code)
	end
	self.handler[self.win_event.keydown] = function(this, code, scan, flags)
		if code < 48 then UIExt.refresh(CurrentScene.layers.pe2d, code | 0x20000) end
	end
	self.handler[self.win_event.timer] = function(this, id)
		if id == 8 then
			UIExt.kill_timer(8)
			UIExt.set_timer(9, 33)
		elseif id == 9 then
			UIExt.paint()
		end
	end
end

function M:init_menu(info)
	local bg = LinearLayout:new({
		padleft = 10,
		padtop = 60,
		padright = 10,
		padbottom = 50
	})
	self:add(bg)
	self.layers.pe2d = bg:add(PE2D:new({
		hit = function(this, evt)
			this.cur = UIExt.refresh(CurrentScene.layers.pe2d, -102)
			UIExt.refresh(CurrentScene.layers.pe2d, CurrentCursorX | 0x40000)
			UIExt.refresh(CurrentScene.layers.pe2d, CurrentCursorY | 0x80000)
			UIExt.refresh(CurrentScene.layers.pe2d, evt | 0x100000)
		end
	}))
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

	-- SLIDER #1
	local slider = LinearLayout:new({
		align = 1,
		padleft = 2,
		padtop = 2,
		padright = 2,
		padbottom = 2,
		pre_resize = function(this, left, top, right, bottom)
			return left, bottom - 50, right - 200, bottom
		end
	})
	self:add(slider)
	Button:new({
		text = '日志',
		font_family = '楷体',
		track_display = 0,
		font_size = 16,
		click = function(this)
			UIExt.refresh(CurrentScene.layers.pe2d, -103)
			UIExt.paint()
		end
	}):attach(slider)
	Button:new({
		text = '暂停',
		font_family = '楷体',
		track_display = 0,
		font_size = 16,
		click = function(this)
			UIExt.refresh(CurrentScene.layers.pe2d, -100)
			UIExt.paint()
		end
	}):attach(slider)
	Button:new({
		text = '重启',
		font_family = '楷体',
		track_display = 0,
		font_size = 16,
		click = function(this)
			UIExt.refresh(CurrentScene.layers.pe2d, -101)
			UIExt.paint()
		end
	}):attach(slider)

end

return M