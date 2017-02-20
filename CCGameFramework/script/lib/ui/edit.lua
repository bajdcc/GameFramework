local GdiBase = require('script.lib.ui.gdibase')

local modname = 'GdiEdit'
local M = GdiBase:new()
_G[modname] = M
package.loaded[modname] = M

function M:new(o)
	o = o or GdiBase:new(o)
	o.type = 1200
	o.color = o.color or '#000000'
	o.size = o.size or 48
	o.family = o.family or 'ËÎÌו'
	o.text = o.text or ''
	o.bold = o.bold or 0
	o.italic = o.italic or 0
	o.underline = o.underline or 0
	setmetatable(o, self)
	self.__index = self
	return o;
end

return M