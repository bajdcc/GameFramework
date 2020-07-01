#include "stdafx.h"
#include "Gdi.h"

CColor::CColor() : r(0), g(0), b(0), a(0xFF)
{

}

CColor::CColor(Gdiplus::ARGB _value, BYTE a) : value(a << 24 | _value)
{

}

CColor::CColor(unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _a /*= 0xFF*/)
    : r(_r), g(_g), b(_b), a(_a)
{

}

cint CColor::Compare(CColor color) const
{
    return value - color.value;
}

CColor CColor::Parse(const CStringA& value)
{
    if (value.Left(4).CompareNoCase("rgba") == 0) {
        static std::regex re(R"(rgba\((\d{1,3}),(\d{1,3}),(\d{1,3},(\d{1,3})\))", std::regex::ECMAScript | std::regex::icase);
        std::smatch m;
        auto s = std::string(value.GetString());
        CColor c;
        if (std::regex_match(s, m, re)) {
            c.r = (BYTE)min(max(atoi(m[1].str().c_str()), 0), 255);
            c.g = (BYTE)min(max(atoi(m[2].str().c_str()), 0), 255);
            c.b = (BYTE)min(max(atoi(m[3].str().c_str()), 0), 255);
            c.a = (BYTE)min(max(atoi(m[4].str().c_str()), 0), 255);
        }
        return c;
    }
    else if (value.Left(4).CompareNoCase("argb") == 0) {
        static std::regex re(R"(argb\((\d{1,3}),(\d{1,3}),(\d{1,3},(\d{1,3})\))", std::regex::ECMAScript | std::regex::icase);
        std::smatch m;
        auto s = std::string(value.GetString());
        CColor c;
        if (std::regex_match(s, m, re)) {
            c.a = (BYTE)min(max(atoi(m[1].str().c_str()), 0), 255);
            c.r = (BYTE)min(max(atoi(m[2].str().c_str()), 0), 255);
            c.g = (BYTE)min(max(atoi(m[3].str().c_str()), 0), 255);
            c.b = (BYTE)min(max(atoi(m[4].str().c_str()), 0), 255);
        }
        return c;
    }
    else if (value.Left(3).CompareNoCase("rgb") == 0) {
        static std::regex re(R"(rgb\((\d{1,3}),(\d{1,3}),(\d{1,3})\))", std::regex::ECMAScript | std::regex::icase);
        std::smatch m;
        auto s = std::string(value.GetString());
        CColor c;
        if (std::regex_match(s, m, re)) {
            c.r = (BYTE)min(max(atoi(m[1].str().c_str()), 0), 255);
            c.g = (BYTE)min(max(atoi(m[2].str().c_str()), 0), 255);
            c.b = (BYTE)min(max(atoi(m[3].str().c_str()), 0), 255);
        }
        return c;
    }
    auto code = "0123456789ABCDEF";
    if ((value.GetLength() == 7 || value.GetLength() == 9) && value[0] == '#')
    {
        cint index[8] = { 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF };
        for (cint i = 0; i < value.GetLength() - 1; i++)
        {
            index[i] = strchr(code, std::toupper(value[i + 1])) - code;
            if (index[i] < 0 || index[i] > 0xF)
            {
                return CColor();
            }
        }

        CColor c;
        c.r = (BYTE)(index[0] * 16 + index[1]);
        c.g = (BYTE)(index[2] * 16 + index[3]);
        c.b = (BYTE)(index[4] * 16 + index[5]);
        c.a = (BYTE)(index[6] * 16 + index[7]);
        return c;
    }
    return CColor();
}

bool CColor::operator==(CColor color) const
{
    return Compare(color) == 0;
}

bool CColor::operator!=(CColor color) const
{
    return Compare(color) != 0;
}

bool CColor::operator<(CColor color) const
{
    return Compare(color) < 0;
}

bool CColor::operator<=(CColor color) const
{
    return Compare(color) <= 0;
}

bool CColor::operator>(CColor color) const
{
    return Compare(color) > 0;
}

bool CColor::operator>=(CColor color) const
{
    return Compare(color) >= 0;
}

CStringA CColor::ToString() const
{
    LPCSTR code = "0123456789ABCDEF";
    char result[] = "#00000000";
    result[1] = code[r / 0x10];
    result[2] = code[r % 0x10];
    result[3] = code[g / 0x10];
    result[4] = code[g % 0x10];
    result[5] = code[b / 0x10];
    result[6] = code[b % 0x10];
    if (a == 0xFF)
    {
        result[7] = '\0';
    }
    else
    {
        result[7] = code[a / 0x10];
        result[8] = code[a % 0x10];
    }
    return result;
}