#ifndef GUI_D2D_H
#define GUI_D2D_H

#include "../bochs.h"
#include "base/utils.h"
#include "ui/gdi/Gdi.h"

class bx_d2d_gui_c : public bx_gui_c {
public:
    bx_d2d_gui_c(void);
    DECLARE_GUI_VIRTUAL_METHODS();
    virtual void statusbar_setitem_specific(int element, bx_bool active, bx_bool w);
    virtual void get_capabilities(Bit16u *xres, Bit16u *yres, Bit16u *bpp);
    virtual void set_tooltip(unsigned hbar_id, const char *tip);
    virtual void set_mouse_mode_absxy(bx_bool mode);
private:
    BYTE * buffer{ nullptr };
    CSize size;
    BOOL error{ FALSE };

public:
    static void AddKeyboardEvent(Bit32u);
    static BOOL GetKeyboardEvent(Bit32u&);

private:
    static std::threadsafe_queue<Bit32u> kbd_events;
};

#endif