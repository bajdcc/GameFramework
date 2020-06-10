//
// Project: clibjs
// Created by bajdcc
//

#ifndef CLIBJS_CJS_H
#define CLIBJS_CJS_H

#include <string>
#include "ui/gdi/Gdi.h"
#include "cjsruntime.h"

namespace clib {

    class cjs {
    public:
        cjs();
        ~cjs();

        cjs(const cjs&) = delete;
        cjs& operator=(const cjs&) = delete;

        void add_stat(const CString& s, bool show = true);

        int exec(const std::string& filename, const std::string& input);

        void paint_window(const CRect& bounds);
        void reset_ips();
        void hit(int n);
        bool try_input(int c, bool ch = true);
        int cursor() const;
        void clear_cache();

        bool run(int cycle, int& cycles);
        int get_state() const;
        void set_state(int);

    private:
        void init_lib();

    private:
        cjsruntime rt;
        int stat_n{ 0 };
        std::list<CString> stat_s;
    };
}

#endif //CLIBJS_CJS_H
