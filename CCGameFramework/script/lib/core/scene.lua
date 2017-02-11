local modname = 'Scene'
local M = {}
_G[modname] = M
package.loaded[modname] = M

M.win_event = {
	created = 1,
    moving = 2,
    moved = 3,
    enabled = 4,
    disabled = 5,
    gotfocus = 6,
    lostfocus = 7,
    activated = 8,
    deactivated = 9,
    opened = 10,
    closing = 11,
    closed = 12,
    paint = 13,
    destroying = 14,
    destroyed = 15,
	timer = 100,
	leftbuttondown = 200,
	leftbuttonup = 201,
    leftbuttondoubleclick = 202,
    rightbuttondown = 203,
    rightbuttonup = 204,
    rightbuttondoubleclick = 205,
    middlebuttondown = 206,
    middlebuttonup = 207,
    middlebuttondoubleclick = 208,
    horizontalwheel = 209,
    verticalwheel = 210,
    mousemove = 211,
	mouseenter = 212,
	mouseleave = 213,
	mousehover = 214,
	keydown = 300,
    keyup = 301,
    syskeydown = 302,
    syskeyup = 303,
    char = 304,
}

function M:new(o)
	o = o or {layers={}, roots={}}
	setmetatable(o, self)
	self.__index = self
	self:initevt()
	return o
end

function M:add(o)
	self.roots[#self.roots + 1] = o
	o.handle = UIExt.add_obj(o)
	o:update()
	return o
end

function M:event(id, ...)
	local f = self.handler[id]
	if f then 
		f(self, ...)
	end
end

M.resize = function(this)
	local info = UIExt.info()
	for k, v in pairs(this.roots) do
		v.resize(v, 0, 0, info.width, info.height)
	end
	UIExt.paint()
end

function M:hittest(x, y)
	local obj = nil
	for k, v in pairs(self.roots) do
		obj = v:hittest(x, y)
		if obj then
			return obj
		end
	end
	return nil
end

function M:initevt()
	self.state = {focused=nil, hover=nil}
	self.handler = {
		[self.win_event.mousemove] = function(this, x, y, flags, wheel)
			local obj = this:hittest(x, y)
			if obj then
				if this.state.hover then
					if this.state.hover ~= obj then
						this.state.hover:hit(this.win_event.mouseleave)
						this.state.hover = obj
						obj:hit(this.win_event.mouseenter)
					end
				else
					this.state.hover = obj
					obj:hit(this.win_event.mouseenter)
				end
				obj:hit(this.win_event.mousemove)
			else
				if this.state.hover then
					this.state.hover:hit(this.win_event.mouseleave)
					this.state.hover = nil
				end
			end
		end,
		[self.win_event.mouseenter] = function(this)
		end,
		[self.win_event.mouseleave] = function(this)
			if this.state.hover then
				this.state.hover:hit(this.win_event.mousemove)
			end
		end,
		[self.win_event.mousehover] = function(this)
			if this.state.hover then
				this.state.hover:hit(this.win_event.mousehover)
			end
		end,
		[self.win_event.leftbuttondown] = function(this, x, y, flags, wheel)
			local obj = this:hittest(x, y)
			if obj then
				if this.state.focused then
					if this.state.focused ~= obj then
						this.state.focused:hit(this.win_event.lostfocus)
						this.state.focused = obj
						obj:hit(this.win_event.gotfocus)
					end
				else
					this.state.focused = obj
					obj:hit(this.win_event.gotfocus)
				end
				obj:hit(this.win_event.leftbuttondown)
			else
				if this.state.focused then
					this.state.focused:hit(this.win_event.lostfocus)
					this.state.focused = nil
				end
			end
		end,
		[self.win_event.leftbuttonup] = function(this, x, y, flags, wheel)
			local obj = this:hittest(x, y)
			if obj then
				obj:hit(this.win_event.leftbuttonup)
			end
		end,
		[self.win_event.leftbuttondoubleclick] = function(this, x, y, flags, wheel)
			local obj = this:hittest(x, y)
			if obj then
				obj:hit(this.win_event.leftbuttondoubleclick)
			end
		end,
		[self.win_event.rightbuttondown] = function(this, x, y, flags, wheel)
			local obj = this:hittest(x, y)
			if obj then
				obj:hit(this.win_event.rightbuttondown)
			end
		end,
		[self.win_event.rightbuttonup] = function(this, x, y, flags, wheel)
			local obj = this:hittest(x, y)
			if obj then
				obj:hit(this.win_event.rightbuttonup)
			end
		end,
		[self.win_event.rightbuttondoubleclick] = function(this, x, y, flags, wheel)
			local obj = this:hittest(x, y)
			if obj then
				obj:hit(this.win_event.rightbuttondoubleclick)
			end
		end,
		[self.win_event.middlebuttondown] = function(this, x, y, flags, wheel)
			local obj = this:hittest(x, y)
			if obj then
				obj:hit(this.win_event.middlebuttondown)
			end
		end,
		[self.win_event.middlebuttonup] = function(this, x, y, flags, wheel)
			local obj = this:hittest(x, y)
			if obj then
				obj:hit(this.win_event.middlebuttonup)
			end
		end,
		[self.win_event.middlebuttondoubleclick] = function(this, x, y, flags, wheel)
			local obj = this:hittest(x, y)
			if obj then
				obj:hit(this.win_event.middlebuttondoubleclick)
			end
		end,
		[self.win_event.char] = function(this, code, flags)
		end,
		[self.win_event.moved] = self.resize
	}
end

return M