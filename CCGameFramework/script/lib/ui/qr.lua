local GdiBase = require('script.lib.ui.gdibase')

local modname = 'GdiQR'
local M = GdiBase:new()
_G[modname] = M
package.loaded[modname] = M

M.type = 1100
M.color = M.color or '#FFFFFF'
M.text = M.text or 'https://github.com/bajdcc/GameFramework'
M.opacity = M.opacity or 1.0

return M