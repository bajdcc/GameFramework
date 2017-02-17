local modname = 'GdiBase'
local M = {}
_G[modname] = M
package.loaded[modname] = M

function M:new(o)
	o = o or {}
	o.handle = o.handle or -1
	o.parent = o.parent or -1
	o.children = o.children or {}
	o.left = o.left or 0
	o.top = o.top or 0
	o.right = o.right or 0
	o.bottom = o.bottom or 0
	o.show_self = o.show_self or true
	o.show_children = o.show_children or true
	o.ht = HitTest.nodecision
	o.cur = SysCursor.arrow
	setmetatable(o, self)
	self.__index = self
	return o;
end

function M:update()
	UIExt.update(self)
end

function M:update_and_paint()
	UIExt.update(self)
	UIExt.paint()
end

function M:attach(parent)
	parent:add(self)
end

function M:hittest(x, y)
	if x >= self.left and x <= self.right and y >= self.top and y <= self.bottom then
		if self.hit then
			return self
		end
		local obj = nil
		for k, v in pairs(self.children) do
			obj = v:hittest(x, y)
			if obj then
				return obj
			end
		end
	end
	return nil
end

function M:pre_resize(left, top, right, bottom)
	return left, top, right, bottom
end

function M:resize(left, top, right, bottom)
	left, top, right, bottom = self:pre_resize(left, top, right, bottom)
	local padleft = self.padleft or 0
	local padtop = self.padtop or 0
	local padright = self.padright or 0
	local padbottom = self.padbottom or 0
	self.left, self.top, self.right, self.bottom = left + padleft, top + padtop, right - padright, bottom - padbottom
	UIExt.update(self)
end

return M