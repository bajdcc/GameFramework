local GdiBase = require('script.lib.ui.block')

local modname = 'GdiLayout'
local M = GdiBase:new()
_G[modname] = M
package.loaded[modname] = M

M.is_layout = true
M.children = {}

function M:add(o)
	self.children[#self.children + 1] = o
	o.parent = self.handle
	o.handle = UIExt.add_obj(o)
	o:update()
	return o
end

M.resize = function(self, left, top, right, bottom)
	self.left, self.top, self.right, self.bottom = left, top, right, bottom
	UIExt.update(self)
	for k, v in pairs(self.children) do
		v.resize(v, left, top, right, bottom)
	end
end

return M