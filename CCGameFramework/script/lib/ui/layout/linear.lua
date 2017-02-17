local GdiBase = require('script.lib.ui.layout.layoutbase')

local modname = 'AbsoluteLayout'
local M = GdiBase:new()
_G[modname] = M
package.loaded[modname] = M

function M:new(o)
	o = o or GdiBase:new(o)
	o.children = {}
	o.align_type = {
		none = 0,
		horizen = 1,
		vertical = 2
	}
	o.align = o.align or o.align_type.none
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
	if #self.children == 0 then return end
	if self.align == self.align_type.none then
		for k, v in pairs(self.children) do
			-- auto-scaling relative
			local releft = v.rel_left or 0.0
			local retop = v.rel_top or 0.0
			local rewidth = v.rel_width or 1.0
			local reheight = v.rel_height or 1.0
			releft = self.left + width * releft
			retop = self.top + height * retop
			local reright = releft + rewidth * width
			local rebottom = retop + reheight * height
			v:resize(releft, retop, reright, rebottom)
		end
	elseif self.align == self.align_type.horizen then
		local n = #self.children;
		local w = (self.right - self.left) / n
		for k, v in pairs(self.children) do
			-- horizen align
			v:resize(self.left + (k - 1) * w, self.top, self.left + k * w, self.bottom)
		end
	elseif self.align == self.align_type.vertical then
		local n = #self.children;
		local h = (self.bottom - self.top) / n
		for k, v in pairs(self.children) do
			-- vertical align
			v:resize(self.left, self.top + (k - 1) * h, self.right, self.top + k * h)
		end
	end
end

return M