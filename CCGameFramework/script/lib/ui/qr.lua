local GdiBase = require('script.lib.ui.gdibase')

local modname = 'GdiQR'
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
	o.type = 1100
	o.color = o.color or '#FFFFFF'
	o.text = o.text or 'https://github.com/bajdcc/GameFramework'
	o.opacity = o.opacity or 1.0

	setmetatable(o, self)
	self.__index = self
	return o;
end

return M