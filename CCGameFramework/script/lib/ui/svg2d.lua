local GdiBase = require('script.lib.ui.gdibase')

local modname = 'GdiSvg'
local M = GdiBase:new()
_G[modname] = M
package.loaded[modname] = M

function M:new(o)
	o = o or GdiBase:new(o)
	o.type = 1109
	o.text = o.text or ''
	o.opacity = o.opacity or 1.0
	setmetatable(o, self)
	self.__index = self
	return o;
end

return M