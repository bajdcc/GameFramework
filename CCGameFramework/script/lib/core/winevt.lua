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
	httpget = 400,
	httppost = 401
}

_G["WinEvent"] = M

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

local S = {
	arrow = 1,
	hand = 2,
	ibeam = 3
}

_G["SysCursor"] = S

local K = {
	backspace = 8,
	enter = 13,
	left = 37,
	up = 38,
	right = 39,
	down = 40
}

_G["SysKey"] = K

local U = {
	toggle_play = 10,
	get_status = 11,
	get_info = 12,
	get_play_info = 13,
	get_sec = 14,
	playing = 15,
	play_loop = 20
}

_G["MusicCtrl"] = U

return M