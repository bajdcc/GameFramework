local modname = 'WinEvent'

local M = {
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
    mousemove = 211,
	mouseenter = 212,
	mouseleave = 213,
	mousehover = 214,
	keydown = 300,
    keyup = 301,
    syskeydown = 302,
    syskeyup = 303,
    char = 304,
}

_G[modname] = M
package.loaded[modname] = M

return M