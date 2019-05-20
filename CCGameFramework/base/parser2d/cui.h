//
// Project: CParser
// Created by bajdcc
//

#ifndef CPARSER_CUI_H
#define CPARSER_CUI_H

class cgui_op {
public:
    virtual bool ready() const = 0;
    virtual void move_to(int x, int y) = 0;
    virtual void line_to(int x, int y) = 0;
    virtual void draw_point(int x, int y) = 0;
    virtual int get_width() const = 0;
    virtual int get_height() const = 0;
    virtual void set_color(uint c) = 0;
    virtual void clear(uint c) = 0;
    virtual void fill_rect(int x, int y) = 0;
    virtual int set_fresh(int fresh) = 0;
};

#endif //CPARSER_CUI_H
