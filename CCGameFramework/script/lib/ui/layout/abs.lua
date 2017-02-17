local GdiBase = require('script.lib.ui.layout.layoutbase')

local modname = 'AbsoluteLayout'
local M = GdiBase:new()
_G[modname] = M
package.loaded[modname] = M

function M:new(o)
	o = o or GdiBase:new(o)
	o.children = {}
	setmetatable(o, self)
	self.__index = self
	return o;
end

return M