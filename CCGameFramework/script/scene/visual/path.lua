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

local modname = 'script.scene.visual.path'
local M = Scene:new()
_G[modname] = M
package.loaded[modname] = M

function M:new(o)
	o = o or {}
	o.name = 'Path Scene'
	o.def = {
		ui = o,
		bgcolor = '#8C8C80',
		blkcolor = {
			[1] = '#E0E0D8', --unchecked
			[2] = '#8C8C80', --barrier
			[3] = '#FF8080', --start
			[4] = '#7171E0', --end
			[5] = '#585858', --reserved
			-- HUE BLUE->BLACK [6]+100
		},
		row = 20,
		col = 40,
		map = {},
		pstart = {x= 10, y= 5},
		pend = {x= 10, y= 35},
		hue = 20,
		elapse = 30,
		type = 2,
		timerid = 10,
		stack = {},
		geval = {}
	}
	setmetatable(o, self)
	self.__index = self
	return o
end

function M:init()
	self.minw = 800
	self.minh = 600
	UIExt.set_minw(self.minw, self.minh)

	UIExt.trace('Scene [Path Page] init')
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
	UIExt.trace('Scene [Path Page]: create background #' .. self.layers.bg.handle)
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
	UIExt.trace('Scene [Path Page]: create text #' .. self.layers.cc.handle)
	-- TEXT
	local text = Text:new({
		color = '#222222',
		text = '寻路算法可视化',
		family = '楷体',
		pre_resize = function(this, left, top, right, bottom)
			return left, top, right, top + 50
		end
	})
	self.layers.text = self:add(text)
	UIExt.trace('Scene [Path Page]: create text #' .. self.layers.text.handle)
	-- MENU
	self:init_menu(info)

	-- EVENT
	self:init_event()

	self.resize(self)
	UIExt.paint()
end

function M:destroy()
	UIExt.trace('Scene [Path Page] destroy')
	UIExt.clear_scene()
end

function M:init_event()
	self.handler[self.win_event.created] = function(this)
		UIExt.trace('Scene [Path Page] Test created message!')
	end
	self.handler[self.win_event.timer] = function(this, id)
		if id == this.def.timerid then
			path_calc(this.def)
		end
	end
	self.handler[self.win_event.keydown] = function(this, code, flags)
	end
end

function M:init_menu(info)
	path_init(self.def)
	path_restart(self.def)
end

-- PATH LOGIC

function path_init(state)
	math.randomseed(os.time())
	math.random(1,10000)
	local row = state.row
	local col = state.col
	local bg = LinearLayout:new({
		padleft = 10,
		padtop = 60,
		padright = 10,
		padbottom = 60
	})
	state.ui:add(bg)
	bg:add(Block:new({
		color = state.bgcolor
	}))
	local menu = TableLayout:new({
		row = row,
		col = col,
		padleft = 1,
		padtop = 1,
		padright = 1,
		padbottom = 1
	})
	state.ui.layers.menu = bg:add(menu)
	for i=1,row*col do
		-- NORMAL BLOCK
		local tile = LinearLayout:new({
			padleft = 1,
			padtop = 1,
			padright = 1,
			padbottom = 1,
		})
		tile.x = (i - 1) / row + 1
		tile.y = (i - 1) % col + 1
		tile:attach(state.ui.layers.menu)
		Block:new({
			color = state.blkcolor[1]
		}):attach(tile)
	end
	local score = Text:new({
		color = '#222222',
		text = '',
		size = 24,
		pre_resize = function(this, left, top, right, bottom)
			return left, bottom - 50, left + 200, bottom
		end
	})
	state.ui.layers.score = state.ui:add(score)
	local slider = LinearLayout:new({
		align = 1,
		padleft = 2,
		padtop = 2,
		padright = 2,
		padbottom = 2,
		pre_resize = function(this, left, top, right, bottom)
			return left, bottom - 50, left + 300, bottom
		end
	})
	state.ui:add(slider)
	Button:new({
		text = 'DFS',
		font_family = '楷体',
		track_display = 0,
		font_size = 16,
		click = function()
			CurrentScene.def.type = 1
			path_restart(CurrentScene.def)
		end
	}):attach(slider)
	Button:new({
		text = 'BFS',
		font_family = '楷体',
		track_display = 0,
		font_size = 16,
		click = function()
			CurrentScene.def.type = 2
			path_restart(CurrentScene.def)
		end
	}):attach(slider)
		Button:new({
		text = 'A*',
		font_family = '楷体',
		track_display = 0,
		font_size = 16,
		click = function()
			CurrentScene.def.type = 3
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
	state.ui.layers.rtstatus = slider.children[#slider.children]
end

function path_restart(state)
	path_init_map(state)
	path_paint(state)
	path_end(state)
	path_start(state)
end

function path_start(state)
	UIExt.set_timer(state.timerid, state.elapse)
	state.stack = {}
	if state.type == 3 then
		local v = math.abs(state.pend.x-state.pstart.x) + math.abs(state.pend.y-state.pstart.y)
		table.insert(state.stack, {v, state.pstart})
	else
		table.insert(state.stack, state.pstart)
	end
end

function path_end(state)
	UIExt.kill_timer(state.timerid)
end

function path_paint(state)
	for i=1,state.row*state.col do
		local k = state.map[i]
		local obj = state.ui.layers.menu.children[i]
		path_update_tile(state, obj, k)
	end
	if state.type == 1 then --DFS
		state.ui.layers.rtstatus.text = '深度 ' .. #state.stack
		state.ui.layers.rtstatus:update()
	elseif state.type == 2 then --BFS
		state.ui.layers.rtstatus.text = '广度 ' .. #state.stack
		state.ui.layers.rtstatus:update()
	elseif state.type == 3 then --A*
		state.ui.layers.rtstatus.text = 'A* ' .. #state.stack
		state.ui.layers.rtstatus:update()
	end
	UIExt.paint()
end

function path_update_tile(state, obj, k)
	if k < 6 then
		obj.children[1].color = state.blkcolor[k]
	else
		obj.children[1].color = UIExt.hsb2rgb(state.hue, 128, 200 - k)
	end
	obj.children[1]:update()
end

function path_setx(map, col, pt, k)
	map[col*(pt.x-1)+pt.y] = k
end

function path_getx(map, col, pt)
	return map[col*(pt.x-1)+pt.y]
end

function path_set_point(state, pt, k)
	path_setx(state.map, state.col, pt, k)
end

function path_get_point(state, pt)
	return path_getx(state.map, state.col, pt)
end

function path_init_map(state)
	for i=1,state.row*state.col do
		state.map[i] = 1
		state.geval[i] = 99999
	end
	path_set_point(state, state.pstart, 3)
	path_set_point(state, state.pend, 4)
	path_set_geval(state, state.pstart, 0)
	path_add_barriar(state)
end

function path_add_barriar_x(state, x, y1, y2)
	local pt = {x= x, y= y1}
	for i=y1,y2 do
		pt.y = i
		path_set_point(state, pt, 2)
	end
end

function path_add_barriar_y(state, y, x1, x2)
	local pt = {x= x1, y= y}
	for i=x1,x2 do
		pt.x = i
		path_set_point(state, pt, 2)
	end
end

function path_add_barriar(state)
	path_add_barriar_x(state, 6, 10, 20)
	path_add_barriar_x(state, 14, 10, 20)
	path_add_barriar_y(state, 20, 7, 13)
end

function path_add_point(pt1, pt2)
	return {x= pt1.x+pt2.x, y= pt1.y+pt2.y}
end

function path_equal_point(pt1, pt2)
	return pt1.x == pt2.x and pt1.y == pt2.y
end

function path_valid_point(state, pt)
	return pt.x > 0 and pt.y > 0 and pt.x <= state.row and pt.y <= state.col
end

function path_get_vector()
	return {
		{x= 0, y= 1},
		{x= 1, y= 0},
		{x= 0, y= -1},
		{x= -1, y= 0}
	}
end

function path_set_geval(state, pt, k)
	return path_setx(state.geval, state.col, pt, k)
end

function path_get_geval(state, pt)
	return path_getx(state.geval, state.col, pt)
end

function path_calc_distance(state, pt)
	if state.type == 3 then
		return 5 + math.ceil(path_get_geval(state, pt))
	else
		local offx, offy = pt.x - state.pstart.x, pt.y - state.pstart.y
		local dis = 3 * math.sqrt(offx*offx + offy*offy)
		return 5 + math.ceil(dis)
	end
end

function path_add_barrial_tracing(state, pt1, pt2)
	local xabs, yabs = math.abs(pt1.x - pt2.x), math.abs(pt1.y - pt2.y)
	if xabs == 0 or yabs == 0 then return 0 end
	local xr, yr = math.ceil(math.random(1,xabs)), math.ceil(math.random(1,yabs))
	local v = path_get_point(state, {x= xr, y= yr})
	if v ~= 1 then return 1 else return 0 end
end

function path_calc(state)
	if #state.stack == 0 then
		path_end(state)
		return
	end
	if state.type == 3 then
		local pt = state.stack[#state.stack]
		table.remove(state.stack)
		local curV = pt[1]
		local cur = pt[2]
		local p = path_get_point(state, cur)
		if p == 5 then
			path_set_point(state, cur, path_calc_distance(state, cur))
		end
		local ge = path_get_geval(state, cur)
		local newp
		for i,v in ipairs(path_get_vector()) do
			newp = path_add_point(cur, v)
			if path_valid_point(state, newp) then
				local m = path_get_point(state, newp)
				if m == 1 then
					local ve = ge
					path_set_geval(state, newp, ge + 1)
					ve = ve + math.abs(state.pend.x-newp.x) + math.abs(state.pend.y-newp.y)
					for j=1,4 do
						ve = ve + path_add_barrial_tracing(state, newp, state.pend)
					end
					table.insert(state.stack, {ve, newp})
					path_set_point(state, newp, 5)
				elseif m == 4 then
					path_end(state)
					return
				end
			end
		end
		table.sort(state.stack, function(a, b) return a[1] > b[1] end)
	else
		local cur = state.stack[#state.stack]
		table.remove(state.stack)
		local p = path_get_point(state, cur)
		if p == 5 then
			path_set_point(state, cur, path_calc_distance(state, cur))
		end
		for i,v in ipairs(path_get_vector()) do
			local newp = path_add_point(cur, v)
			if path_valid_point(state, newp) then
				local m = path_get_point(state, newp)
				if m == 1 then
					if state.type == 1 then --DFS
						table.insert(state.stack, newp)
					elseif state.type == 2 then --BFS
						table.insert(state.stack, 1, newp)
					end
					path_set_point(state, newp, 5)
				elseif m == 4 then
					path_end(state)
					break
				end
			end
		end
	end
	path_paint(state)
end

return M