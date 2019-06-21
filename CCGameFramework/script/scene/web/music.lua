local Scene = require('script.lib.core.scene')
local Gradient = require('script.lib.ui.gradient')
local LinearLayout = require('script.lib.ui.layout.linear')
local Empty = require('script.lib.ui.empty')
local Block = require('script.lib.ui.block')
local Text = require('script.lib.ui.text')
local Button = require('script.lib.ui.comctl.button')
local Edit = require('script.lib.ui.comctl.edit')
local JSON = require("script.lib.core.dkjson")
local Base64Image = require('script.lib.ui.b64img')

local modname = 'script.scene.web.music'
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
		color = '#1F7256',
		right = info.width,
		bottom = info.height
	}))
	UIExt.trace('Scene [Music page]: create background #' .. self.layers.bg.handle)
	-- BG2
	local bg2 = Gradient:new({
		color1 = '#1F7256',
		color2 = '#164032',
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
				if MusicSceneReturn ~= nil then
					FlipScene(MusicSceneReturn)
					MusicSceneReturn = nil
					return
				end
				FlipScene('Button')
			end
		end
	})
	self.layers.cc = self:add(cc)
	UIExt.trace('Scene [Music page]: create text #' .. self.layers.cc.handle)
	-- TEXT
	local text = Text:new({
		color = '#EEEEEE',
		text = '在线听歌',
		family = '楷体',
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
	for i,v in ipairs(self.layers.btns.children) do
		v.text = ''
	end
	UIExt.clear_scene()
end

local function music_set_text(obj, text)
	if text == nil then return end
	obj.text = text
	obj:update()
end

function M:init_event()
	self.handler[self.win_event.created] = function(this)
		UIExt.trace('Scene [Music page] Test created message!')
	end
	self.handler[self.win_event.timer] = function(this, id)
		if id == 15 then
			music_set_text(this.layers.ctrl.children[3], UIExt.music_ctrl(MusicCtrl.get_status))
			this.progress = UIExt.music_ctrl(MusicCtrl.get_progress)
			this.layers.bar:_resize()
			UIExt.paint()
		elseif id == 16 then
			music_set_text(this.layers.ctrl.children[4], UIExt.music_ctrl(MusicCtrl.get_info))
			UIExt.paint()
		elseif id == 17 then
			music_set_text(this.layers.rtstatus, UIExt.music_ctrl(MusicCtrl.get_play_info))
			UIExt.paint()
		elseif id == 18 then
			if UIExt.music_ctrl(MusicCtrl.playing) and this.lyric ~= nil then
				local ly = this.lyric[UIExt.music_ctrl(MusicCtrl.get_sec)]
				if ly then
					music_set_text(this.layers.lyric, ly)
				end
			else
				UIExt.kill_timer(18)
				music_set_text(this.layers.lyric, '')
			end
			UIExt.paint()
		elseif id == 20 then
			UIExt.paint()
			if MusicSceneReturn ~= nil then
				FlipScene(MusicSceneReturn)
				MusicSceneReturn = nil
				return
			end
			UIExt.music_ctrl(MusicCtrl.play_loop)
			UIExt.set_timer(18, 100)
		end
	end
	self.handler[self.win_event.httpget] = function(this, id, code, text)
		if id == 10 then
			if code ~= 200 then
				music_set_text(this.layers.rtstatus, '歌曲下载失败')
				return
			end
			music_set_text(this.layers.rtstatus, '准备播放歌曲')
			
			UIExt.set_timer(15, 1000)
			UIExt.set_timer(16, 2000)
			UIExt.set_timer(17, 1500)
			UIExt.set_timer(18, 100)
			UIExt.set_timer(20, 10000)

			music_set_text(this.layers.ctrl.children[2], this.song_name .. ' - ' .. this.singer)
			music_set_text(this.layers.idstatus, 'ID：' .. this.song_id )
			music_set_text(this.layers.lyric, '')

			UIExt.play_song(text)

			this.layers.bar.percent = 0.0
			this.layers.vol:update_vol(UIExt.music_ctrl(MusicCtrl.get_vol))
			UIExt.paint()
		elseif id == 11 then
			if code ~= 200 then
				music_set_text(this.layers.rtstatus, '封面下载失败')
				return
			end
			music_set_text(this.layers.rtstatus, '封面加载成功')
			this.layers.pic.text = text
			this.layers.pic:update()
		elseif id == 12 then
			local obj = JSON.decode(text, 1, nil)
			if obj == nil or obj.code ~= 200 or obj.lyric == nil then
				music_set_text(this.layers.rtstatus, '歌词下载失败')
				return
			end
			music_set_text(this.layers.rtstatus, '歌词加载成功')
			this.lyric = UIExt.parse_lyric(obj.lyric)
			music_set_text(this.layers.lyric, '正在播放歌曲：' .. this.song_name)
			UIExt.paint()
		end
	end
	self.handler[self.win_event.httppost] = function(this, id, code, text)
		if id == 8 then
			if code ~= 200 or text == nil then
				music_set_text(this.layers.rtstatus, '获取列表失败')
				return
			end
			music_set_text(this.layers.rtstatus, '获取列表成功')
			local obj = JSON.decode(text, 1, nil)
			if obj == nil or obj.code ~= 200 or obj.result == nil or obj.result.songs == nil then return end
			this.sids = obj.result.songs
			local btns = this.layers.btns.children
			for i,v in ipairs(obj.result.songs) do
				btns[i]:reset(UIExt.limit_str(v.name, 25))
			end
			if MusicSceneId ~= nil then
				btns[MusicSceneId]:click()
				MusicSceneId = nil
			end
			UIExt.paint()
		elseif id == 9 then
			if code ~= 200 or text == nil then
				music_set_text(this.layers.rtstatus, '获取信息失败')
				return
			end
			music_set_text(this.layers.rtstatus, '歌曲下载中')
			local obj = JSON.decode(text, 1, nil)
			if obj == nil or obj.code ~= 200 or obj.songs == nil then
				music_set_text(this.layers.rtstatus, '歌曲解析失败')
				UIExt.paint()
				return
			end
			if #obj.songs == 0 or obj.songs[1] == nil then
				music_set_text(this.layers.rtstatus, '没有播放源')
				UIExt.paint()
				return
			end
			local song = obj.songs[1]
			local url = song['mp3Url']
			this.song_name = song['name']
			this.singer = song.artists[1]['name']
			this.pic_url = song['album']['picUrl']
			this.song_id = song['id']
			if url == nil then
				url = 'http://music.163.com/song/media/outer/url?id=' .. this.song_id .. '.mp3'
			end
			if url ~= nil and url ~= '' then
				Web.getb(url, 10)
				this.layers.pic.url = this.pic_url
				Web.getb(this.pic_url, 11)
				Web.get('http://music.163.com/api/song/media?id=' .. this.song_id, 12)
			else
				music_set_text(this.layers.rtstatus, '播放地址无效')
				UIExt.paint()
			end
		end
	end
end

function M:init_menu(info)
	self.sids = {}
	self.playid = 0
	self.progress = 0.0

	local bar = Block:new({
		color = '#F5441E',
		pre_resize = function(this, left, top, right, bottom)
			return left, top, right * CurrentScene.progress, 4
		end,
		_resize = function(this)
			local info = UIExt.info()
			this:resize(0, 0, info.width, info.height)
		end
	})
	self.layers.bar = self:add(bar)

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
			return 10, h - 120, 200, h + 200
		end
	})
	self.layers.ctrl = self:add(ctrl)
	
	Button:new({
		text = '播放/暂停',
		track_display = 0,
		click = function()
			UIExt.music_ctrl(MusicCtrl.toggle_play)
		end
	}):attach(ctrl)

	Text:new({
		text = '',
		color = '#EEEEEE',
		family = '楷体',
		size = 20,
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

	-- ALBUM PIC

	local b64 = Base64Image:new({
		pre_resize = function(this, left, top, right, bottom)
			local w = left + (right - left) / 2
			local h = top + (bottom - top) / 2
			return w + 220, h - 120, right - 10, bottom - 80
		end
	})
	self.layers.pic = self:add(b64)

	-- LYRIC
	local lyric = Text:new({
		color = '#EEEEEE',
		text = '',
		family = '楷体',
		size = 32,
		pre_resize = function(this, left, top, right, bottom)
			return left, top + 130, right, top + 160
		end
	})
	self.layers.lyric = self:add(lyric)
	UIExt.trace('Scene [Music page]: create lyric #' .. self.layers.lyric.handle)
	
	-- MENU BUTTON
	Edit:new({
		text = MusicSceneName or '',
		char_return = function (text)
		end,
		char_input = function (text)
			Web.post('http://music.163.com/api/search/suggest/web', 8, 's=' .. text)
		end
	}):attach(menu)

	if MusicSceneName ~= nil then
		Web.post('http://music.163.com/api/search/suggest/web', 8, 's=' .. MusicSceneName)
		MusicSceneName = nil
	end

	Empty:new():attach(menu)

	-- STATUS
	local slider = LinearLayout:new({
		align = 2,
		padleft = 2,
		padtop = 2,
		padright = 2,
		padbottom = 2,
		pre_resize = function(this, left, top, right, bottom)
			return left, bottom - 120, left + 120, bottom
		end
	})
	self:add(slider)

	Text:new({
		text = '',
		color = '#EEEEEE',
		size = 16
	}):attach(slider)
	self.layers.idstatus = slider.children[#slider.children]

	Text:new({
		text = '',
		family = '楷体',
		color = '#EEEEEE',
		size = 16
	}):attach(slider)
	self.layers.rtstatus = slider.children[#slider.children]

	-- VOLUME
	local volp = LinearLayout:new({
		align = 2,
		padleft = 2,
		padtop = 2,
		padright = 2,
		padbottom = 2,
		pre_resize = function(this, left, top, right, bottom)
			return left + 130, bottom - 60, left + 250, bottom
		end
	})
	self:add(volp)
	self.layers.vol = Edit:new({
		text = '',
		readonly = 1,
		char_return = function (text)
		end,
		char_input = function (this, code)
			local c = string.char(code)
			if c == '=' then
				if CurrentScene.vol <= 90 then
					this:update_vol(CurrentScene.vol + 10)
				end
			elseif c == '-' then
				if CurrentScene.vol >= 10 then
					this:update_vol(CurrentScene.vol - 10)
				end
			end
		end,
		font_size = 18
	})
	local vol = self.layers.vol
	self.vol = 0
	vol.update_vol = function(this, v)
		CurrentScene.vol = v
		this:reset('音量：' .. v)
		UIExt.music_ctrl(MusicCtrl.set_vol, v)
		UIExt.paint()
	end
	self.layers.vol:attach(volp)
end

function M:play_song(id)
	if id == 0 or id > 4 then return end
	local i = self.sids[id]
	if i == nil then return end
	local id = i['id']
	if id == nil then return end
	music_set_text(self.layers.rtstatus, '分析歌曲信息')
	
	UIExt.kill_timer(15)
	UIExt.kill_timer(16)
	UIExt.kill_timer(17)
	UIExt.kill_timer(18)
	UIExt.kill_timer(20)

	Web.post('http://music.163.com/api/song/detail', 9, 'id=' .. id .. '&ids=[' .. id .. ']')

	UIExt.paint()
end

return M