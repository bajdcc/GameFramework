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

M.resize = M.resize or function(self, left, top, right, bottom)
	self.left, self.top, self.right, self.bottom = left, top, right, bottom
	UIExt.update(self)
end

return M