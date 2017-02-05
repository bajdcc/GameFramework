local modname = 'Scene'
local M = {}
_G[modname] = M
package.loaded[modname] = M

M.win_event = {
	created = 1,
    moving = 2,
    moved = 3,
    enabled = 4,
    disabled = 5,
    gotfocus = 6,
    lostfocus = 7,
    activated = 8,
    deactivated = 9,
    opened = 10,
    closing = 11,
    closed = 12,
    paint = 13,
    destroying = 14,
    destroyed = 15,
	timer = 100,
	leftbuttondown = 200,
	leftbuttonup = 201,
    leftbuttondoubleclick = 202,
    rightbuttondown = 203,
    rightbuttonup = 204,
    rightbuttondoubleclick = 205,
    middlebuttondown = 206,
    middlebuttonup = 207,
    middlebuttondoubleclick = 208,
    horizontalwheel = 209,
    verticalwheel = 210,
    mousemoving = 211,
	keydown = 300,
    keyup = 301,
    syskeydown = 302,
    syskeyup = 303,
    char = 304,
}

function M:new(o)
	o = o or {layers={}}
	setmetatable(o, self)
	self.__index = self
	return o
end

function M:add(o)
	o.handle = UIExt.add_obj(o)
	o:update()
	return o
end

function M:event(id, ...)
	local f = self.handler[id]
	if f then 
		f(self, ...)
	end
end

return M