local GdiBase = require('script.lib.ui.gdibase')

local modname = 'GdiBlock'
local M = GdiBase:new()
_G[modname] = M
package.loaded[modname] = M

M.type = 1001
M.color = M.color or '#FFFFFF'

return M