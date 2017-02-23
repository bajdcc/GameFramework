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

local modname = 'Game2048Scene'
local M = Scene:new()
_G[modname] = M
package.loaded[modname] = M

function M:new(o)
	o = o or {}
	o.name = '2048 - Game Scene'
	o.state = {focused=nil, hover=nil}
	o.gamedef = {
		size = 4,
		display = {
			[0] = '',
			[1] = '2',
			[2] = '4',
			[3] = '8',
			[4] = '16',
			[5] = '32',
			[6] = '64',
			[7] = '128',
			[8] = '256',
			[9] = '512',
			[10] = '1024',
			[11] = '2048'
		},
		bgcolor = '#BBADA0',
		color = {
			[0] = '#776E65',
			[1] = '#776E65',
			[2] = '#776E65',
			[3] = '#F9F6F2',
			[4] = '#F9F6F2',
			[5] = '#F9F6F2',
			[6] = '#F9F6F2',
			[7] = '#F9F6F2',
			[8] = '#F9F6F2',
			[9] = '#F9F6F2',
			[10] = '#F9F6F2',
			[11] = '#F9F6F2'
		},
		bg = {
			[0] = '#CDC1B4',
			[1] = '#EEE4DA',
			[2] = '#EDE0C8',
			[3] = '#F2B179',
			[4] = '#F59563',
			[5] = '#F67C5F',
			[6] = '#F65E3B',
			[7] = '#EDCF72',
			[8] = '#EDCC61',
			[9] = '#EDC850',
			[10] = '#EDC53F',
			[11] = '#EDC22E'
		},
		fontsize = {
			[0] = '48',
			[1] = '48',
			[2] = '48',
			[3] = '48',
			[4] = '40',
			[5] = '40',
			[6] = '40',
			[7] = '32',
			[8] = '32',
			[9] = '32',
			[10] = '24',
			[11] = '24'
		},
		map = {},
		ui = o,
		startnum = 2
	}
	setmetatable(o, self)
	self.__index = self
	return o;
end

function M:init()
	self.minw = 800
	self.minh = 600
	UIExt.set_minw(self.minw, self.minh)

	UIExt.trace('Scene [2048 Game Page] init')
	-- INFO
	local info = UIExt.info()
	-- BG
	local bg = AbsoluteLayout:new({
		right = info.width,
		bottom = info.height
	})
	self.layers.bg = self:add(bg)
	bg:add(Block:new({
		color = '#EEEEEE',
		right = info.width,
		bottom = info.height
	}))
	UIExt.trace('Scene [2048 Game Page]: create background #' .. self.layers.bg.handle)
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
	UIExt.trace('Scene [2048 Game Page]: create text #' .. self.layers.cc.handle)
	-- TEXT
	local text = Text:new({
		color = '#222222',
		text = '2048',
		pre_resize = function(this, left, top, right, bottom)
			return left, top, right, bottom / 2
		end
	})
	self.layers.text = self:add(text)
	UIExt.trace('Scene [2048 Game Page]: create text #' .. self.layers.text.handle)
	-- MENU
	self:init_menu(info)

	-- EVENT
	self:init_event()

	self.resize(self)
	UIExt.paint()
end

function M:destroy()
	UIExt.trace('Scene [2048 Game Page] destroy')
	UIExt.clear_scene()
end

function M:init_event()
	self.handler[self.win_event.created] = function(this)
		UIExt.trace('Scene [2048 Game Page] Test created message!')
	end
	self.handler[self.win_event.timer] = function(this, id)
	end
end

function M:init_menu(info)
	game_init(self.gamedef)
	game_restart(self.gamedef)
end

-- GAME LOGIC
function game_init(state)
	math.randomseed(os.time())
	math.random(1,10000)
	local row = 4
	local col = 4
	local bg = LinearLayout:new({
		pre_resize = function(this, left, top, right, bottom)
			local w = left + (right - left) / 2
			local h = top + (bottom - top) / 2
			return w - 150, h - 50, w + 150, h + 250
		end
	})
	state.ui:add(bg)
	bg:add(Radius:new({
		color = state.bgcolor,
		radius = 5
	}))
	local menu = TableLayout:new({
		row = row,
		col = col,
		padleft = 5,
		padtop = 5,
		padright = 5,
		padbottom = 5
	})
	bg:add(menu)
	state.ui.layers.menu = menu
	for i=1,state.size*state.size do
		state.map[i] = 0
		local tile = LinearLayout:new({
				padleft = 5,
				padtop = 5,
				padright = 5,
				padbottom = 5
		})
		tile.x = (i - 1) / state.size + 1
		tile.y = (i - 1) % state.size + 1
		tile:attach(state.ui.layers.menu)
		Radius:new({
			color = state.bg[0],
			radius = 4
		}):attach(tile)
		Text:new({
			color = state.color[0],
			text = state.display[0],
			size = 40,
			bold = 1
		}):attach(tile)
	end
end

function game_paint(state)
	for i=1,state.size*state.size do
		local k = state.map[i]
		local obj = state.ui.layers.menu.children[i]
		obj.children[1].color = state.bg[k]
		obj.children[1]:update()
		obj.children[2].color = state.color[k]
		obj.children[2].text = state.display[k]
		obj.children[2].size = state.fontsize[k]
		obj.children[2]:update()
	end
	UIExt.paint()
end

function game_restart(state)
	for i=1,state.startnum do
		game_add_tile(state)
	end
	game_paint(state);
end

function game_add_tile(state)
	local newi
	if math.random(1,10) == 1 then newi = 4 else newi = 2 end
	local newk = game_get_random_available_cell(state)
	state.map[newk] = newi
end

function game_get_available_cells(state)
	local cells = {}
	for k,v in ipairs(state.map) do
		if v == 0 then
			cells[#cells + 1] = k
		end
	end
	return cells
end

function game_get_random_available_cell(state)
	local cells = game_get_available_cells(state)
	local r = math.random(1, #cells)
	return cells[r]
end

return M