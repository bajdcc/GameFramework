//
// Project: clibjs
// Created by bajdcc
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <cassert>
#include <sstream>
#include <ui\gdi\Gdi.h>
#include "cjs.h"
#include "cjsparser.h"
#include "cjsgen.h"
#include "cjsgui.h"

#define LIBRARY_FILE R"(lib/clib.js)"

#define STAT_DELAY_N 60
#define STAT_MAX_N 10

namespace clib {

    cjs::cjs() {
        rt.set_readonly(false);
        rt.init();
        init_lib();
    }

    cjs::~cjs()
    {
        rt.destroy();
    }

    int cjs::exec(const std::string& filename, const std::string& input)
    {
        return rt.exec(filename, input);
    }

    void cjs::resize()
    {
        rt.send_signal("resize");
    }

    void cjs::paint_window(const CRect& bounds)
    {
    }

    void cjs::reset_ips()
    {
    }

    void cjs::hit(int n)
    {
    }

    bool cjs::try_input(int c, bool ch)
    {
        return true;
    }

    int cjs::cursor() const
    {
        return 0;
    }

    void cjs::clear_cache()
    {
        rt.clear_cache();
    }

    bool cjs::run(int cycle, int& cycles)
    {
        return rt.run_internal(cycle, cycles) != 11;
    }

    int cjs::get_state() const
    {
        return rt.get_state();
    }

    void cjs::set_state(int n) 
    {
        return rt.set_state(n);
    }

    int cjs::get_frame() const
    {
        return rt.get_frame();
    }

    void cjs::clear_frame()
    {
        rt.clear_frame();
    }

    void cjs::change_target()
    {
        rt.change_target();
    }

    void cjs::init_lib() {
        char buf[256];
        snprintf(buf, sizeof(buf), "sys.exec_file(\"%s\");\n", LIBRARY_FILE);
        rt.exec("<library>", buf);
    }
}