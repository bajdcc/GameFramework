local GdiBase = require('script.lib.ui.gdibase')

local modname = 'GdiText'
local M = GdiBase:new()
_G[modname] = M
package.loaded[modname] = M

M.Alignment = {
	Near = 0,
	Center = 1,
	Far = 2
}

M.type = 1002
M.color = M.color or '#000000'
M.size = M.size or 48
M.family = M.family or 'ËÎÌו'
M.align = M.align or M.Alignment.Center
M.valign = M.valign or M.Alignment.Center
M.text = M.text or ''

return M