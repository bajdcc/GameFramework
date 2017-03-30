local Scene = require('script.lib.core.scene')
local Gradient = require('script.lib.ui.gradient')
local Block = require('script.lib.ui.block')
local Radius = require('script.lib.ui.radius')
local Text = require('script.lib.ui.text')
local QR = require('script.lib.ui.qr')
local Base64Image = require('script.lib.ui.b64img')
local JSON = require("script.lib.core.dkjson")

local modname = 'script.scene.web.hitokoto'
local M = Scene:new()
_G[modname] = M
package.loaded[modname] = M

function M:new(o)
	o = o or {}
	o.name = 'Hitokoto Scene'
	o.state = {focused=nil, hover=nil}
	setmetatable(o, self)
	self.__index = self
	return o
end

function M:init()
	UIExt.trace('Scene [Hitokoto page] init')
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
	UIExt.trace('Scene [Hitokoto page]: create background #' .. self.layers.bg.handle)
	local b64 = Base64Image:new({
		right = info.width,
		bottom = info.height,
		opacity = 0.8
	})
	self.layers.b64 = self:add(b64)
	local titlebg = Radius:new({
		color = '#111111AA',
		radius = 10,
		padleft = 100,
		padright = 100,
		pre_resize = function(this, left, top, right, bottom)
			local height = top + (bottom - top) / 2
			return left, height - 70, right, height + 70
		end
	})
	self.layers.titlebg = self:add(titlebg)
	local title = Text:new({
		color = '#FFFFFF',
		text = 'Ò»ÑÔ - Hitokoto',
		pre_resize = function(this, left, top, right, bottom)
			return left, top, right, bottom / 2
		end
	})
	self.layers.title = self:add(title)
	-- TEXT
	local text = Text:new({
		color = '#EEEEEE',
		text = 'Ò»ÑÔ- ¥Ò¥È¥³¥È - Hitokoto.us',
		family = '¿¬Ìå',
		right = info.width,
		bottom = info.height,
		size = 24,
		padleft = 30,
		padright = 30,
	})
	self.layers.text = self:add(text)
	UIExt.trace('Scene [Hitokoto page]: create text #' .. self.layers.text.handle)
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
	UIExt.trace('Scene [Hitokoto page]: create text #' .. self.layers.cc.handle)
	-- QR
	local qr = QR:new({
		text = 'https://github.com/bajdcc/GameFramework',
		color = '#111111',
		pre_resize = function(this, left, top, right, bottom)
			return right - 170, bottom - 190, right - 30, bottom - 50
		end,
		hit = function(this, evt)
			if evt == WinEvent.gotfocus then
				this.color = '#005098'
				this:update()
			elseif evt == WinEvent.lostfocus then
				this.color = '#111111'
				this:update()
			elseif evt == WinEvent.mouseenter then
				this.opacity = 0.7
				this:update()
			elseif evt == WinEvent.mouseleave then
				this.opacity = 1.0
				this:update()
			end
		end
	})
	self.layers.qr = self:add(qr)
	UIExt.trace('Scene [Hitokoto page]: create qr #' .. self.layers.qr.handle)

	-- EVENT
	self:init_event()

	-- TIMER
	UIExt.set_timer(2, 8000)
	UIExt.set_timer(5, 10000)

	self.bgidx = 0

	self.resize(self)
	UIExt.paint()
end

function M:destroy()
	UIExt.trace('Scene [Hitokoto page] destroy')
	UIExt.clear_scene()
end

function M:init_event()
	self.handler[self.win_event.created] = function(this)
		UIExt.trace('Scene [Hitokoto page] Test created message!')
	end
	self.handler[self.win_event.timer] = function(this, id)
		if id == 5 then
			Web.get('http://www.bing.com/HPImageArchive.aspx?format=js&n=1&idx=' .. this.bgidx, 101)
		elseif id == 2 then
			--Web.get('http://api.hitokoto.us/rand?cat=a&length=30', 100)
			Web.get('http://api.hitokoto.cn/?c=a&length=30&encode=json', 100)
		end
	end
	self.handler[self.win_event.httpget] = function(this, id, code, text)
		if id == 100 then
			local obj = JSON.decode(text, 1, nil)
			if obj ~= nil then
				--local disp = obj.hitokoto .. ' \n      ¡ª¡ª¡º' .. obj.source .. '¡»'
				local disp = '¡º' .. obj.from .. '¡»\n\n' .. obj.hitokoto
				this.layers.text.text = disp
				this.layers.text:update_and_paint()
			end
		elseif id == 101 then
			local obj = JSON.decode(text, 1, nil)
			if obj ~= nil then
				local imgurl = 'http://www.bing.com' .. obj.images[1].url
				this.bgidx = this.bgidx + 1
				if this.bgidx > 6 then this.bgidx = 0 end
				this.layers.b64.url = imgurl
				this.layers.b64:update()
				Web.getb(imgurl, 102)
			end
		elseif id == 102 then
			this.layers.b64.text = text
			this.layers.b64:update_and_paint()
		end
	end
end

return M