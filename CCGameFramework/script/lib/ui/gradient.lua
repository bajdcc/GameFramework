local GdiBase = require('script.lib.ui.gdibase')

local modname = 'GdiGradient'
local M = GdiBase:new()
_G[modname] = M
package.loaded[modname] = M

M.Direction = {
	Horizontal = 0,
	Vertical = 1,
	Slash = 2,
	Backslash = 3,
}

M.type = 1003
M.color1 = M.color1 or '#FFFFFF'
M.color2 = M.color2 or '#000000'
M.direction = M.direction or M.Direction.Horizontal

return M