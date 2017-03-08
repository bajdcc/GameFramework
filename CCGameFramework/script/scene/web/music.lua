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
end

function M:play_song(id)
	local id = self.sids[id].id
	Web.post('http://music.163.com/api/song/detail', 9, 'id=' .. id .. '&ids=[' .. id .. ']')
end

return M