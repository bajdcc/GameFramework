local GdiBase = require('script.lib.ui.layout.linear')
local Text = require('script.lib.ui.text')
local Radius = require('script.lib.ui.radius')

local modname = 'GdiButton'
local M = GdiBase:new()
_G[modname] = M
package.loaded[modname] = M

function M:new(o)
	o = o or GdiBase:new(o)
	o.children = {}
	o.bgcolor = o.bgcolor or '#CCCCCC'
	o.bgcolor_focus = o.bgcolor_focus or '#FFFFFF'
	o.fgcolor = o.fgcolor or '#333333'
	o.fgcolor_focus = o.fgcolor_focus or '#000000'
	o.text = o.text or 'Button'
	o.font_size = o.font_size or 24
	o.padleft = o.padleft or 10
	o.padtop = o.padtop or 10
	o.padright = o.padright or 10
	o.padbottom = o.padbottom or 10
	o.layers = {}
	o.track_display = o.track_display or 1
	o.cur = SysCursor.hand
	o.radius = o.radius or 10
	o.state = {focused=false, enter=false}
	o.hit = function(this, evt)
		if evt == WinEvent.leftbuttonup then
			this.layers.bg.color = this.bgcolor
			this.layers.bg:update_and_paint()
			if this.click then this.click() end
		elseif evt == WinEvent.gotfocus then
			this.state.focused = true
			this.layers.bg.color = this.bgcolor_focus
			this.layers.bg:update_and_paint()
		elseif evt == WinEvent.lostfocus then
			this.state.focused = false
			this.layers.bg.color = this.bgcolor
			this.layers.bg:update_and_paint()
		elseif evt == WinEvent.mouseenter then
			this.state.enter = true
			if this.track_display ~= 0 then
				this.layers.fg.text = '[ ' .. this.text .. ' ]'
			end
			this.layers.fg.color = this.fgcolor_focus
			this.layers.fg:update_and_paint()
		elseif evt == WinEvent.mouseleave then
			this.state.enter = false
			this.layers.fg.text = this.text
			this.layers.fg.color = this.fgcolor
			this.layers.fg:update_and_paint()
		end
	end
	setmetatable(o, self)
	self.__index = self
	return o;
end

function M:attach(parent)
	parent:add(self)
	self:add(Radius:new({
		color = self.bgcolor,
		radius = self.radius
	}))
	self:add(Text:new({
		color = self.fgcolor,
		text = self.text,
		size = self.font_size
	}))
	self.layers.bg = self.children[1]
	self.layers.fg = self.children[2]
end

return M