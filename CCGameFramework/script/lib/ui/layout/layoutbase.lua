local GdiBase = require('script.lib.ui.empty')

local modname = 'GdiLayout'
local M = GdiBase:new()
_G[modname] = M
package.loaded[modname] = M

function M:new(o)
	o = o or GdiBase:new(o)
	o.is_layout = true
	o.children = {}
	setmetatable(o, self)
	self.__index = self
	return o
end

function M:add(o)
	self.children[#self.children + 1] = o
	o.parent = self.handle
	o.handle = UIExt.add_obj(o)
	o:update()
	return o
end

function M:pre_resize(left, top, right, bottom)
	return left, top, right, bottom
end

function M:resize(left, top, right, bottom)
	left, top, right, bottom = self:pre_resize(left, top, right, bottom)
	local padleft = self.padleft or 0
	local padtop = self.padtop or 0
	local padright = self.padright or 0
	local padbottom = self.padbottom or 0
	self.left, self.top, self.right, self.bottom = left + padleft, top + padtop, right - padright, bottom - padbottom
	UIExt.update(self)
	for k, v in pairs(self.children) do
		v:resize(self.left, self.top, self.right, self.bottom)
	end
end

return M