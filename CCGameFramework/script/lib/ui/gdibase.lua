local modname = 'GdiBase'
local M = {}
_G[modname] = M
package.loaded[modname] = M

function M:new(o)
	o = o or {
		handle = -1,
		left = 0,
		top = 0,
		right = 0,
		bottom = 0
	}
	setmetatable(o, self)
	self.__index = self
	return o;
end

return M