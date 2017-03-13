local GdiBase = require('script.lib.ui.layout.linear')
local AbsoluteLayout = require('script.lib.ui.layout.abs')
local Edit = require('script.lib.ui.edit')
local Block = require('script.lib.ui.Block')

local modname = 'GdiEdit'
local M = GdiBase:new()
_G[modname] = M
package.loaded[modname] = M

function M:new(o)
	o = o or GdiBase:new(o)
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
	o.readonly = o.readonly or 0
	o.hit = function(this, evt, args)
		if evt == WinEvent.leftbuttonup then
			if this.click then this.click() end
		elseif evt == WinEvent.gotfocus then
			this.state.focused = true
			this.layers.border.color = this.bordercolor_focus
			this.layers.border:update_and_paint()
		elseif evt == WinEvent.lostfocus then
			this.state.focused = false
			this.layers.border.color = this.bordercolor
			this.layers.border:update_and_paint()
		elseif evt == WinEvent.mouseenter then
			this.state.enter = true
			this.layers.border.color = this.bordercolor_enter
			this.layers.border:update_and_paint()
		elseif evt == WinEvent.mouseleave then
			this.state.enter = false
			if this.state.focused then
				this.layers.border.color = this.bordercolor_focus
			else
				this.layers.border.color = this.bordercolor
			end
			this.layers.border:update_and_paint()
		elseif evt == WinEvent.char then
			if this.readonly ~= 0 then
				if this.char_input then this.char_input(this, args.code) end
				return
			end
			if UIExt.isprintable(args.code) then
				this.text = this.text .. UIExt.import_char(args.code)
				this.layers.text.text = this.text
			elseif args.code == SysKey.backspace then
				this.text = UIExt.remove_char(this.text)
				this.layers.text.text = this.text 
			elseif args.code == SysKey.enter then
				if this.multiline ~= 0 then
					this.text = this.text .. '\n'
					this.layers.text.text = this.text
				else
					if this.char_return then this.char_return(this.text) end
				end
			end
			if this.char_input then this.char_input(this.text) end
			this.layers.text:update_and_paint()
		end
	end
	setmetatable(o, self)
	self.__index = self
	return o
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
		show_children = 0
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

function M:reset(text)
	self.text = text
	self.layers.text.text = text
	self.layers.text:update()
end

return M