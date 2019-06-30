//
// Project: CParser
// Created by bajdcc
//

#ifndef CPARSER_CWND_H
#define CPARSER_CWND_H

#include "types.h"
#include <ui\window\Window.h>

namespace clib {
    class cwindow {
    public:
        explicit cwindow(const string_t& caption, const CRect& location);
        ~cwindow();

        void paint(const CRect& bounds);

    private:
        void init();

    private:
        string_t caption;
        CRect location;
        std::shared_ptr<IGraphicsElement> root;
        std::shared_ptr<Direct2DRenderTarget> renderTarget;
        CRect bounds1, bounds2;

        struct SystemBag {
            std::shared_ptr<IGraphicsElement> title, title_text;
        } bag;
    };
}

#endif //CPARSER_CWND_H
