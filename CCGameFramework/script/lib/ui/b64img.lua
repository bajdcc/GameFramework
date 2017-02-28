local GdiBase = require('script.lib.ui.gdibase')

local modname = 'GdiBase64Image'
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
	o.type = 1101
	o.text = o.text or ''
	o.url = o.url or ''
	o.opacity = o.opacity or 1.0

	setmetatable(o, self)
	self.__index = self
	return o;
end

return M