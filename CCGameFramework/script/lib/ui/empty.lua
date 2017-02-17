local GdiBase = require('script.lib.ui.gdibase')

local modname = 'GdiEmpty'
local M = GdiBase:new()
_G[modname] = M
package.loaded[modname] = M

function M:new(o)
	o = o or GdiBase:new(o)
	o.type = 1000
	setmetatable(o, self)
	self.__index = self
	return o
end

return M