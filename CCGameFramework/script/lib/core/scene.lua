local modname = 'Scene'
local M = {}
_G[modname] = M
package.loaded[modname] = M

function M:new(o)
	o = o or {layers={}}
	setmetatable(o, self)
	self.__index = self
	return o
end

function M:add(o)
	o.handle = UIExt.add_obj(o)
	UIExt.update(o)
	return o.handle
end

function M:FlipScene()
	if CurrentScene ~= nil then
		CurrentScene:destroy()
	end
	CurrentScene = self
	CurrentScene:init()
end

return M