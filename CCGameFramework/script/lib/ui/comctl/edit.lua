local GdiBase = require('script.lib.ui.layout.linear')
local AbsoluteLayout = require('script.lib.ui.layout.abs')
local Edit = require('script.lib.ui.edit')
local Block = require('script.lib.ui.Block')

local modname = 'GdiEdit'
local M = GdiBase:new()
_G[modname] = M
package.loaded[modname] = M

function M:new(o)
	o = o or GdiBase:new()
	o.children = {}
	o.bgcolor = o.bgcolor or '#FFFFFF'
	o.bgcolor_focus = o.bgcolor_focus or '#ADD6E1'
	o.bordercolor = o.bordercolor or '#B8B8B8'
	o.bordercolor_enter = o.bordercolor_enter or '#999999'
	o.bordercolor_focus = o.bordercolor_focus or '#4791E1'
	o.textcolor = o.textcolor or '#000000'
	o.text = o.text or 'Edit'
	o.font_size = o.font_size or 24
	o.font_family = o.font_family or 'ËÎÌו'
	o.padleft = o.padleft or 10
	o.padtop = o.padtop or 10
	o.padright = o.padright or 10
	o.padbottom = o.padbottom or 10
	o.multiline = o.multiline or 0
	o.layers = {}
	o.cur = SysCursor.ibeam
	o.state = {focused=false, enter=false}
	o.hit = function(this, evt, args)
		if evt == WinEvent.leftbuttonup then
			if o.click then o.click() end
		elseif evt == WinEvent.gotfocus then
			o.state.focused = true
			this.layers.border.color = this.bordercolor_focus
			this.layers.border:update_and_paint()
		elseif evt == WinEvent.lostfocus then
			o.state.focused = false
			this.layers.border.color = this.bordercolor
			this.layers.border:update_and_paint()
		elseif evt == WinEvent.mouseenter then
			o.state.enter = true
			this.layers.border.color = this.bordercolor_enter
			this.layers.border:update_and_paint()
		elseif evt == WinEvent.mouseleave then
			o.state.enter = false
			if o.state.focused then
				this.layers.border.color = this.bordercolor_focus
			else
				this.layers.border.color = this.bordercolor
			end
			this.layers.border:update_and_paint()
		elseif evt == WinEvent.char then
			if UIExt.isprintable(args.code) then
				this.layers.text.text = this.layers.text.text .. string.char(args.code)
			elseif args.code == SysKey.backspace then
				this.layers.text.text = string.sub(this.layers.text.text, 0, -2)
			elseif args.code == SysKey.enter then
				if o.multiline ~= 0 then
					this.layers.text.text = this.layers.text.text .. '\n'
				else
					if o.char_return then o.char_return(this.layers.text.text) end
				end
			end
			if o.char_input then o.char_input(this.layers.text.text) end
			this.layers.text:update_and_paint()
		end
	end
	setmetatable(o, self)
	self.__index = self
	return o;
end

function M:attach(parent)
	parent:add(self)
	self.layers.bg = self:add(Block:new({ -- BG
		color = self.bgcolor
	}))
	self.layers.border = self:add(Block:new({ -- BORDER
		color = self.bordercolor,
		fill = 0
	}))
	self:add(AbsoluteLayout:new({
		show_children = false
	}))
	self.layers.sel = self.children[3]:add(Block:new({ -- SELECT BG
		color = self.bgcolor_focus,
	}))
	self.layers.text = self:add(Edit:new({ -- TEXT
		color = self.textcolor,
		size = self.font_size,
		family = self.font_family,
		text = self.text,
		multiline = self.multiline,
		padleft = 5,
		padtop = 5,
		padright = 5,
		padbottom = 5,
	}))
end

return M