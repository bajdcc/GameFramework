local Scene = require('script.lib.core.scene')
local Gradient = require('script.lib.ui.gradient')
local LinearLayout = require('script.lib.ui.layout.linear')
local Empty = require('script.lib.ui.empty')
local Block = require('script.lib.ui.block')
local Text = require('script.lib.ui.text')
local Button = require('script.lib.ui.comctl.button')
local Edit = require('script.lib.ui.comctl.edit')
local JSON = require("script.lib.core.dkjson")

local modname = 'WebMusicScene'
local M = Scene:new()
_G[modname] = M
package.loaded[modname] = M

function M:new(o)
	o = o or {}
	o.name = 'Web Music Scene'
	setmetatable(o, self)
	self.__index = self
	return o;
end

function M:init()
	self.minw = 800
	self.minh = 600
	UIExt.set_minw(self.minw, self.minh)

	UIExt.trace('Scene [Music page] init')
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
	UIExt.trace('Scene [Music page]: create background #' .. self.layers.bg.handle)
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
	UIExt.trace('Scene [Music page]: create background2 #' .. self.layers.bg2.handle)
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
				FlipScene('Button')
			end
		end
	})
	self.layers.cc = self:add(cc)
	UIExt.trace('Scene [Music page]: create text #' .. self.layers.cc.handle)
	-- TEXT
	local text = Text:new({
		color = '#EEEEEE',
		text = 'ÔÚÏßÌý¸è',
		pre_resize = function(this, left, top, right, bottom)
			return left, top, right, top + 120
		end
	})
	self.layers.text = self:add(text)
	UIExt.trace('Scene [Music page]: create text #' .. self.layers.text.handle)
	-- MENU
	self:init_menu(info)

	-- EVENT
	self:init_event()

	self.resize(self)
	UIExt.paint()
end

function M:destroy()
	UIExt.trace('Scene [Music page] destroy')
	UIExt.clear_scene()
end

function M:init_event()
	self.handler[self.win_event.created] = function(this)
		UIExt.trace('Scene [Music page] Test created message!')
	end
	self.handler[self.win_event.timer] = function(this, id)
		if id == 15 then
			local progress = UIExt.music_ctrl(MusicCtrl.get_status)
			if progress == nil then return end
			local target = this.layers.ctrl.children[2]
			target.text = progress
			target:update_and_paint()
		elseif id == 16 then
			local info = UIExt.music_ctrl(MusicCtrl.get_info)
			if info == nil then return end
			local target = this.layers.ctrl.children[3]
			target.text = info
			target:update_and_paint()
		elseif id == 17 then
			local info = UIExt.music_ctrl(MusicCtrl.get_play_info)
			if info == nil then return end
			local target = this.layers.rtstatus
			target.text = info
			target:update_and_paint()
		end
	end
	self.handler[self.win_event.httpget] = function(this, id, code, text)
		if id == 10 then
			if code ~= 200 then return end
			UIExt.play_song(text)
		end
	end
	self.handler[self.win_event.httppost] = function(this, id, code, text)
		if id == 8 then
			if code ~= 200 or text == nil then return end
			local obj = JSON.decode(text, 1, nil)
			if obj == nil or obj.code ~= 200 or obj.result == nil or obj.result.songs == nil then return end
			this.sids = obj.result.songs
			for i,v in ipairs(obj.result.songs) do
				local t = this.layers.btns.children[i]
				t.text = v.name
				t.layers.fg.text = v.name
				t.layers.fg:update()
			end
			UIExt.paint()
		elseif id == 9 then
			if code ~= 200 or text == nil then return end
			local obj = JSON.decode(text, 1, nil)
			if obj == nil or obj.code ~= 200 or obj.songs == nil then return end
			if #obj.songs == 0 or obj.songs[1] == nil then return end
			local url = obj.songs[1]['mp3Url']
			if url ~= nil and url ~= '' then
				Web.getb(url, 10)
			end
		end
	end
end

function M:init_menu(info)
	self.sids = {}
	self.playid = 0

	UIExt.set_timer(15, 1000)
	UIExt.set_timer(16, 2000)
	UIExt.set_timer(17, 1500)

	-- MUSIC LIST

	local btns = LinearLayout:new({
		align = 2,
		pre_resize = function(this, left, top, right, bottom)
			local w = left + (right - left) / 2
			local h = top + (bottom - top) / 2
			return w - 200, h - 120, w + 200, bottom - 80
		end
	})
	self.layers.btns = self:add(btns)
	
	Button:new({
		text = '',
		track_display = 0,
		click = function()
			CurrentScene:play_song(1)
		end
	}):attach(btns)

	Button:new({
		text = '',
		track_display = 0,
		click = function()
			CurrentScene:play_song(2)
		end
	}):attach(btns)

	Button:new({
		text = '',
		track_display = 0,
		click = function()
			CurrentScene:play_song(3)
		end
	}):attach(btns)

	Button:new({
		text = '',
		track_display = 0,
		click = function()
			CurrentScene:play_song(4)
		end
	}):attach(btns)

	-- MUSIC CONTROLLER

	local ctrl = LinearLayout:new({
		align = 2,
		pre_resize = function(this, left, top, right, bottom)
			local w = left + (right - left) / 2
			local h = top + (bottom - top) / 2
			return 10, h - 120, 200, h + 120
		end
	})
	self.layers.ctrl = self:add(ctrl)
	
	Button:new({
		text = '²¥·Å/ÔÝÍ£',
		track_display = 0,
		click = function()
			UIExt.music_ctrl(MusicCtrl.toggle_play)
		end
	}):attach(ctrl)

	Text:new({
		text = '',
		color = '#EEEEEE',
		size = 24,
	}):attach(ctrl)

	Text:new({
		text = '',
		color = '#EEEEEE',
		size = 16,
		align = 0,
	}):attach(ctrl)

	-- MENU CONTAINER LAYOUT
	local menu = LinearLayout:new({
		pre_resize = function(this, left, top, right, bottom)
			local w = left + (right - left) / 2
			local h = top + (bottom - top) / 2
			return w - 100, bottom - 60, w + 100, bottom
		end
	})
	self.layers.menu = self:add(menu)
	
	-- MENU BUTTON
	Edit:new({
		text = '',
		char_return = function (text)
			CurrentScene.layers.text.text = text
			CurrentScene.layers.text:update_and_paint()
		end,
		char_input = function (text)
			Web.post('http://music.163.com/api/search/suggest/web', 8, 's=' .. text)
			CurrentScene.layers.text.text = text
			CurrentScene.layers.text:update_and_paint()
		end
	}):attach(menu)

	Empty:new():attach(menu)

	-- STATUS
	local slider = LinearLayout:new({
		align = 1,
		padleft = 2,
		padtop = 2,
		padright = 2,
		padbottom = 2,
		pre_resize = function(this, left, top, right, bottom)
			return left, bottom - 50, left + 120, bottom
		end
	})
	self:add(slider)

	Text:new({
		text = '×´Ì¬',
		family = '¿¬Ìå',
		color = '#EEEEEE',
		size = 16
	}):attach(slider)
	self.layers.rtstatus = slider.children[#slider.children]
end

function M:play_song(id)
	if id == 0 or id > 4 then return end
	local i = self.sids[id]
	if i == nil then return end
	local id = i['id']
	if id == nil then return end
	Web.post('http://music.163.com/api/song/detail', 9, 'id=' .. id .. '&ids=[' .. id .. ']')
end

return M