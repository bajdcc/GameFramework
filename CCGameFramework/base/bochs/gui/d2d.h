#ifndef GUI_D2D_H
#define GUI_D2D_H

#include "../bochs.h"
#include "ui/gdi/Gdi.h"

class bx_d2d_gui_c : public bx_gui_c {
public:
    bx_d2d_gui_c(void);
    DECLARE_GUI_VIRTUAL_METHODS();
    virtual void statusbar_setitem_specific(int element, bx_bool active, bx_bool w);
    virtual void get_capabilities(Bit16u *xres, Bit16u *yres, Bit16u *bpp);
    virtual void set_tooltip(unsigned hbar_id, const char *tip);
    virtual void set_mouse_mode_absxy(bx_bool mode);
#if BX_SHOW_IPS
    virtual void show_ips(Bit32u ips_count);
#endif
private:
    BYTE * buffer{ nullptr };
    CSize size;
    BOOL error{ FALSE };
};

#endif