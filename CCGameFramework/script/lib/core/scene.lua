local modname = 'Scene'
local M = {}
_G[modname] = M
package.loaded[modname] = M

M.win_event = WinEvent

M.minw = 480
M.minh = 320

function M:new(o)
	o = o or {}
	o.layers = {}
	o.roots = {}
	o.state = {focused=nil, hover=nil}
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

function M:resize()
	local info = UIExt.info()
	if info.width < self.minw then
		info.width = self.minw
	end
	if info.height < self.minh then
		info.height = self.minh
	end
	for k, v in pairs(self.roots) do
		v:resize(0, 0, info.width, info.height)
	end
	UIExt.paint()
end

function M:hittest(x, y)
	local obj = nil
	for k, v in pairs(self.roots) do
		obj = v:hittest(x, y)
		if obj then
			CurrentHitTest = obj.ht
			CurrentCursor = obj.cur
			return obj
		end
	end
	CurrentHitTest = HitTest.nodecision
	CurrentCursor = SysCursor.arrow
	return nil
end

function M:initevt()
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
			if this.state.focused ~= nil then
				this.state.focused:hit(this.win_event.char, {['code']=code,['flags']=flags})
			end
		end,
		[self.win_event.moved] = self.resize
	}
end

return M