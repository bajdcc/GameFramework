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
local PE2D = require('script.lib.ui.2dpe')

local modname = 'script.scene.visual.2d'
local M = Scene:new()
_G[modname] = M
package.loaded[modname] = M

function M:new(o)
	o = o or {}
	o.name = '图形学展示'
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

	UIExt.trace('Scene [2D Physics Engine] init')
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
	UIExt.trace('Scene [2D Physics Engine]: create background #' .. self.layers.bg.handle)
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
	UIExt.trace('Scene [2D Physics Engine]: create text #' .. self.layers.cc.handle)
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
	UIExt.trace('Scene [2D Physics Engine]: create text #' .. self.layers.text.handle)
	-- MENU
	self:init_menu(info)

	-- EVENT
	self:init_event()

	-- WATERMARK
	local wm = Text:new({
		color = '#1E75BB',
		text = '点击下方按钮以查看图形学示例',
		family = '楷体',
		pre_resize = function(this, left, top, right, bottom)
			return left, top + 50, right, bottom - 50
		end
	})
	self.layers.wm = self:add(wm)
	UIExt.trace('Scene [2D Physics Engine]: create watermark #' .. self.layers.wm.handle)

	UIExt.set_timer(8, 500)

	self.resize(self)
	UIExt.paint()
end

function M:destroy()
	UIExt.trace('Scene [2D Physics Engine] destroy')
	UIExt.clear_scene()
end

function M:init_event()
	self.handler[self.win_event.created] = function(this)
		UIExt.trace('Scene [2D Physics Engine] Test created message!')
	end
	self.handler[self.win_event.timer] = function(this, id)
		if id == 8 then
			UIExt.kill_timer(8)
			this.layers.pe2d:update_and_paint()
			UIExt.refresh(this.layers.pe2d, 0)
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
	self.layers.pe2d = bg:add(PE2D:new())
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
			return left, bottom - 50, left + 500, bottom
		end
	})
	self:add(slider)
	Button:new({
		text = '色彩图',
		font_family = '楷体',
		track_display = 0,
		font_size = 16,
		click = function(this)
			CurrentScene.layers.text.text = '随机渐变'
			CurrentScene.layers.text:update()
			CurrentScene.layers.wm.show_self = 0
			CurrentScene.layers.wm:update()
			UIExt.refresh(CurrentScene.layers.pe2d, 1)
			UIExt.paint()
		end
	}):attach(slider)
	Button:new({
		text = '球体渲染',
		font_family = '楷体',
		track_display = 0,
		font_size = 16,
		click = function()
			CurrentScene.layers.text.text = '简单球体光线追踪'
			CurrentScene.layers.text:update()
			CurrentScene.layers.wm.show_self = 0
			CurrentScene.layers.wm:update()
			UIExt.refresh(CurrentScene.layers.pe2d, 2)
			UIExt.paint()
		end
	}):attach(slider)
	Button:new({
		text = '添加材质',
		font_family = '楷体',
		track_display = 0,
		font_size = 16,
		click = function()
			CurrentScene.layers.wm.show_self = 0
			CurrentScene.layers.wm:update()
			UIExt.refresh(CurrentScene.layers.pe2d, 3)
			UIExt.paint()
		end
	}):attach(slider)
	Button:new({
		text = '发光的球',
		font_family = '楷体',
		track_display = 0,
		font_size = 16,
		click = function()
			CurrentScene.layers.wm.show_self = 0
			CurrentScene.layers.wm:update()
			UIExt.refresh(CurrentScene.layers.pe2d, 4)
			UIExt.paint()
		end
	}):attach(slider)
	self.layers.rtstatus = slider.children[#slider.children]
end

return M