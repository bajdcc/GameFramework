//
// Project: CParser
// Created by bajdcc
//

#ifndef CPARSER_CUI_H
#define CPARSER_CUI_H

class cgui_op {
public:
    virtual void move_to(int x, int y) = 0;
    virtual void line_to(int x, int y) = 0;
};

#endif //CPARSER_CUI_H
