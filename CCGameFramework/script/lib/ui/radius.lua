local GdiBase = require('script.lib.ui.gdibase')

local modname = 'GdiRadius'
local M = GdiBase:new()
_G[modname] = M
package.loaded[modname] = M

function M:new(o)
	o = o or GdiBase:new(o)
	o.type = 1004
	o.color = o.color or '#FFFFFF'
	o.radius = o.radius or 1.0
	o.fill = o.fill or 1
	setmetatable(o, self)
	self.__index = self
	return o;
end

function M:resize(left, top, right, bottom)
	left, top, right, bottom = self:pre_resize(left, top, right, bottom)
	local padleft = self.padleft or 0
	local padtop = self.padtop or 0
	local padright = self.padright or 0
	local padbottom = self.padbottom or 0
	self.left, self.top, self.right, self.bottom = left + padleft, top + padtop, right - padright, bottom - padbottom
	UIExt.update(self)
end

return M