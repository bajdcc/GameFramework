local GdiBase = require('script.lib.ui.layout.layoutbase')

local modname = 'TableLayout'
local M = GdiBase:new()
_G[modname] = M
package.loaded[modname] = M

function M:new(o)
	o = o or GdiBase:new(o)
	o.children = {}
	o.row = o.row or 2
	o.col = o.col or 2
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

function M:resize(left, top, right, bottom)
	left, top, right, bottom = self:pre_resize(left, top, right, bottom)
	local padleft = self.padleft or 0
	local padtop = self.padtop or 0
	local padright = self.padright or 0
	local padbottom = self.padbottom or 0
	self.left, self.top, self.right, self.bottom = left + padleft, top + padtop, right - padright, bottom - padbottom
	UIExt.update(self)
	local width = self.right - self.left
	local height = self.bottom - self.top
	local n = #self.children;
	if n == 0 then return end
	local w = (self.right - self.left) / self.col
	local h = (self.bottom - self.top) / self.row
	for i=1,self.row do
		for j=1,self.col do
			self.children[(i-1)*self.col+j]:resize(self.left + (j - 1) * w, self.top + (i - 1) * h, self.left + j * w, self.top + i * h)
		end
	end
end

return M