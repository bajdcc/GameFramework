local GdiBase = require('script.lib.ui.gdibase')

local modname = 'GdiText'
local M = GdiBase:new()
_G[modname] = M
package.loaded[modname] = M

function M:new(o)
	o = o or GdiBase:new(o)
	o.Alignment = {
		Near = 0,
		Center = 1,
		Far = 2
	}

	o.type = 1002
	o.color = o.color or '#000000'
	o.size = o.size or 48
	o.family = o.family or 'ËÎÌו'
	o.align = o.align or o.Alignment.Center
	o.valign = o.valign or o.Alignment.Center
	o.text = o.text or ''
	o.italic = o.italic or 0
	o.bold = o.bold or 0
	o.underline = o.underline or 0
	o.strikeline = o.strikeline or 0
	setmetatable(o, self)
	self.__index = self
	return o;
end

return M