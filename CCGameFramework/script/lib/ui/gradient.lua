local GdiBase = require('script.lib.ui.gdibase')

local modname = 'GdiGradient'
local M = GdiBase:new()
_G[modname] = M
package.loaded[modname] = M

function M:new(o)
	o = o or GdiBase:new(o)
	o.Direction = {
		Horizontal = 0,
		Vertical = 1,
		Slash = 2,
		Backslash = 3,
	}
	o.type = 1003
	o.color1 = o.color1 or '#FFFFFF'
	o.color2 = o.color2 or '#000000'
	o.direction = o.direction or o.Direction.Horizontal
	setmetatable(o, self)
	self.__index = self
	return o
end

return M