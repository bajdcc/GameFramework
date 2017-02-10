local GdiBase = require('script.lib.ui.gdibase')

local modname = 'GdiEmpty'
local M = GdiBase:new()
_G[modname] = M
package.loaded[modname] = M

M.type = 1000

return M