local modname = 'GdiBase'
local M = {}
_G[modname] = M
package.loaded[modname] = M

function M:new(o)
	o = o or {
		handle = -1,
		parent = -1,
		children = {},
		left = 0,
		top = 0,
		right = 0,
		bottom = 0,
		show_self = true,
		show_children = true
	}
	setmetatable(o, self)
	self.__index = self
	return o;
end

function M:update()
	UIExt.update(self)
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

M.resize = M.resize or function(self, left, top, right, bottom)
	self.left, self.top, self.right, self.bottom = left, top, right, bottom
	UIExt.update(self)
end

return M