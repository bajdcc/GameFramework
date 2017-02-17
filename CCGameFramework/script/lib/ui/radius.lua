local GdiBase = require('script.lib.ui.gdibase')

local modname = 'GdiRadius'
local M = GdiBase:new()
_G[modname] = M
package.loaded[modname] = M

function M:new(o)
	o = o or GdiBase:new(o)
	o.type = 1004
	o.color = o.color or '#FFFFFF'
	o.radius = o.radius or 1.0
	setmetatable(o, self)
	self.__index = self
	return o;
end

function tracebackex()  
local ret = ""  
local level = 2  
ret = ret .. "stack traceback:\n"  
while true do  
   --get stack info  
   local info = debug.getinfo(level, "Sln")  
   if not info then break end  
   if info.what == "C" then                -- C function  
    ret = ret .. tostring(level) .. "\tC function\n"  
   else           -- Lua function  
    ret = ret .. string.format("\t[%s]:%d in function `%s`\n", info.short_src, info.currentline, info.name or "")  
   end  
   --get local vars  
   local i = 1  
   while true do  
    local name, value = debug.getlocal(level, i)  
    if not name then break end  
    ret = ret .. "\t\t" .. name .. " =\t" .. tostringex(value, 3) .. "\n"  
    i = i + 1  
   end    
   level = level + 1  
end  
return ret  
end  
  
function tostringex(v, len)  
if len == nil then len = 0 end  
local pre = string.rep('\t', len)  
local ret = ""  
if type(v) == "table" then  
   if len > 5 then return "\t{ ... }" end  
   local t = ""  
   for k, v1 in pairs(v) do  
    t = t .. "\n\t" .. pre .. tostring(k) .. ":"  
    t = t .. tostringex(v1, len + 1)  
   end  
   if t == "" then  
    ret = ret .. pre .. "{ }\t(" .. tostring(v) .. ")"  
   else  
    if len > 0 then  
     ret = ret .. "\t(" .. tostring(v) .. ")\n"  
    end  
    ret = ret .. pre .. "{" .. t .. "\n" .. pre .. "}"  
   end  
else  
   ret = ret .. pre .. tostring(v) .. "\t(" .. type(v) .. ")"  
end  
return ret  
end  

function M:resize(left, top, right, bottom)
	left, top, right, bottom = self:pre_resize(left, top, right, bottom)
	local padleft = self.padleft or 0
	local padtop = self.padtop or 0
	local padright = self.padright or 0
	local padbottom = self.padbottom or 0
	self.left, self.top, self.right, self.bottom = left + padleft, top + padtop, right - padright, bottom - padbottom
	UIExt.update(self)
end

return M