#include "stdafx.h"
#include "Gdi.h"

Font::Font()
    : size(0)
    , bold(false)
    , italic(false)
    , underline(false)
    , strikeline(false)
    , antialias(true)
    , verticalAntialias(true)
{

}

cint Font::Compare(const Font& value) const
{
    cint result = 0;

    result = fontFamily.Compare(value.fontFamily);
    if (result != 0) return result;

    result = size - value.size;
    if (result != 0) return result;

    result = (cint)bold - (cint)value.bold;
    if (result != 0) return result;

    result = (cint)italic - (cint)value.italic;
    if (result != 0) return result;

    result = (cint)underline - (cint)value.underline;
    if (result != 0) return result;

    result = (cint)strikeline - (cint)value.strikeline;
    if (result != 0) return result;

    result = (cint)antialias - (cint)value.antialias;
    if (result != 0) return result;

    return 0;
}

bool Font::operator>=(const Font& value) const
{
    return Compare(value) >= 0;
}

bool Font::operator>(const Font& value) const
{
    return Compare(value) > 0;
}

bool Font::operator<=(const Font& value) const
{
    return Compare(value) <= 0;
}

bool Font::operator<(const Font& value) const
{
    return Compare(value) < 0;
}

bool Font::operator!=(const Font& value) const
{
    return Compare(value) != 0;
}

bool Font::operator==(const Font& value) const
{
    return Compare(value) == 0;
}