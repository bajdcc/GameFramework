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

_G["WinEvent"] = M
package.loaded["WinEvent"] = M

local N = {
	bordernosizing = 0,
    borderleft = 1,
    borderright = 2,
    bordertop = 3,
    borderbottom = 4,
    borderlefttop = 5,
    borderrighttop = 6,
    borderleftbottom = 7,
    borderrightbottom = 8,
    title = 9,
    buttonminimum = 10,
    buttonmaximum = 11,
    buttonclose = 12,
    client = 13,
    icon = 14,
    nodecision = 15,
}

_G["HitTest"] = N
package.loaded["HitTest"] = N

local S = {
	arrow = 1,
	hand = 2
}

_G["SysCursor"] = S
package.loaded["SysCursor"] = S

return M