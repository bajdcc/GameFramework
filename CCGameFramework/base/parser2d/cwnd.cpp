//
// Project: CParser
// Created by bajdcc
//

#include "stdafx.h"
#include "cwnd.h"

namespace clib {

    cwindow::cwindow(const string_t& caption, const CRect& location)
        : caption(caption), location(location)
    {
    }

    cwindow::~cwindow()
    {
    }
}