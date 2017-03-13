#include "stdafx.h"
#include "Window.h"
#include <base64/b64.h>
#include "utils.h"

int ui_clear_scene(lua_State *L)
{
    for (auto& timer : window->setTimer)
    {
        ::KillTimer(window->handle, timer);
    }
    window->ptrEle = 0;
    window->mapEle.clear();
    window->setTimer.clear();
    window->root->GetRenderer()->SetRenderTarget(nullptr);
    window->root->GetChildren().clear();
    if (window->zplay)
    {
        window->zplay->Stop();
        window->zplay->Release();
        window->zplay = nullptr;
        delete window->zplaydata;
        window->zplaydata = nullptr;
    }
    return 0;
}

int ui_add_obj(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_getfield(L, -1, "type");
    auto type = ElementId(cint(luaL_checkinteger(L, -1)));
    lua_pop(L, 1);
    window->ptrEle++;
    auto ptr = window->ptrEle;
    while (window->mapEle.find(ptr) != window->mapEle.end())
    {
        ptr++;
        if (ptr > 0x10000000)
            ptr = 0;
    }
    window->ptrEle = ptr;
    lua_getfield(L, -1, "parent");
    auto parent = ElementId(cint(luaL_checkinteger(L, -1)));
    decltype(window->root) p;
    if (parent == -1)
        p = window->root;
    else if (window->mapEle.find(parent) != window->mapEle.end())
        p = window->mapEle[parent].lock();
    else
        p = window->root;
    std::shared_ptr<IGraphicsElement> obj;
    switch (type)
    {
    case Empty:
        obj = EmptyElement::Create();
        break;
    case SolidBackground:
        obj = SolidBackgroundElement::Create();
        break;
    case SolidLabel:
        obj = SolidLabelElement::Create();
        break;
    case GradientBackground:
        obj = GradientBackgroundElement::Create();
        break;
    case RoundBorder:
        obj = RoundBorderElement::Create();
        break;
    case QRImage:
        obj = QRImageElement::Create();
        break;
    case Base64Image:
        obj = Base64ImageElement::Create();
        break;
    case WireworldAutomaton:
        obj = WireworldAutomatonImageElement::Create();
        break;
    case Edit:
        obj = EditElement::Create();
        break;
    default:
        return luaL_argerror(L, 1, "Invalid obj id");
    }
    obj->GetFlags().parent = p;
    p->GetChildren().push_back(obj);
    window->mapEle.insert(std::make_pair(ptr, obj));
    lua_pushinteger(L, ptr);
    return 1;
}

int ui_update_obj(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_getfield(L, -1, "type");
    auto type = ElementId(cint(luaL_checkinteger(L, -1))); lua_pop(L, 1);
    lua_getfield(L, -1, "handle");
    auto handle = cint(luaL_checkinteger(L, -1)); lua_pop(L, 1);
    if (window->mapEle.find(handle) == window->mapEle.end())
        return luaL_argerror(L, 1, "Invalid obj id");
    auto o = window->mapEle[handle].lock();
    if (o->GetTypeId() != type)
        return luaL_argerror(L, 1, "Invalid obj type");
    {
        CRect rt;
        lua_getfield(L, -1, "left");
        rt.left = LONG(luaL_checknumber(L, -1)); lua_pop(L, 1);
        lua_getfield(L, -1, "top");
        rt.top = LONG(luaL_checknumber(L, -1)); lua_pop(L, 1);
        lua_getfield(L, -1, "right");
        rt.right = LONG(luaL_checknumber(L, -1)); lua_pop(L, 1);
        lua_getfield(L, -1, "bottom");
        rt.bottom = LONG(luaL_checknumber(L, -1)); lua_pop(L, 1);
        o->SetRenderRect(rt);
    }
    {
        lua_getfield(L, -1, "show_self");
        o->GetFlags().self_visible = cint(luaL_checkinteger(L, -1)) != 0; lua_pop(L, 1);
        lua_getfield(L, -1, "show_children");
        o->GetFlags().children_visible = cint(luaL_checkinteger(L, -1)) != 0; lua_pop(L, 1);
    }
    switch (type)
    {
    case Empty:
        break;
    case SolidBackground:
    {
        auto obj = static_cast<SolidBackgroundElement*>(o.get());
        {
            lua_getfield(L, -1, "color");
            auto color = luaL_checkstring(L, -1); lua_pop(L, 1);
            obj->SetColor(CColor::Parse(color));
            lua_getfield(L, -1, "fill");
            auto fill = luaL_checknumber(L, -1) != 0; lua_pop(L, 1);
            obj->SetFill(fill);
        }
    }
    break;
    case SolidLabel:
    {
        auto obj = static_cast<SolidLabelElement*>(o.get());
        {
            lua_getfield(L, -1, "color");
            auto color = luaL_checkstring(L, -1); lua_pop(L, 1);
            obj->SetColor(CColor::Parse(color));
        }
        {
            Font font;
            lua_getfield(L, -1, "size");
            font.size = cint(luaL_checkinteger(L, -1)); lua_pop(L, 1);
            lua_getfield(L, -1, "family");
            font.fontFamily = CString(luaL_checkstring(L, -1)); lua_pop(L, 1);
            lua_getfield(L, -1, "italic");
            font.italic = cint(luaL_checkinteger(L, -1)) != 0; lua_pop(L, 1);
            lua_getfield(L, -1, "bold");
            font.bold = cint(luaL_checkinteger(L, -1)) != 0; lua_pop(L, 1);
            lua_getfield(L, -1, "underline");
            font.underline = cint(luaL_checkinteger(L, -1)) != 0; lua_pop(L, 1);
            lua_getfield(L, -1, "strikeline");
            font.strikeline = cint(luaL_checkinteger(L, -1)) != 0; lua_pop(L, 1);
            obj->SetFont(font);
        }
        {
            lua_getfield(L, -1, "text");
            obj->SetText(CString(luaL_checkstring(L, -1))); lua_pop(L, 1);
        }
        {
            lua_getfield(L, -1, "align");
            obj->SetHorizontalAlignment(Alignment(cint(luaL_checkinteger(L, -1)))); lua_pop(L, 1);
            lua_getfield(L, -1, "valign");
            obj->SetVerticalAlignment(Alignment(cint(luaL_checkinteger(L, -1)))); lua_pop(L, 1);
        }
    }
    break;
    case GradientBackground:
    {
        auto obj = static_cast<GradientBackgroundElement*>(o.get());
        {
            lua_getfield(L, -1, "color1");
            auto color1 = luaL_checkstring(L, -1); lua_pop(L, 1);
            obj->SetColor1(CColor::Parse(color1));
            lua_getfield(L, -1, "color2");
            auto color2 = luaL_checkstring(L, -1); lua_pop(L, 1);
            obj->SetColor2(CColor::Parse(color2));
        }
        {
            lua_getfield(L, -1, "direction");
            obj->SetDirection(GradientBackgroundElement::Direction(cint(luaL_checkinteger(L, -1)))); lua_pop(L, 1);
        }
    }
    break;
    case RoundBorder:
    {
        auto obj = static_cast<RoundBorderElement*>(o.get());
        {
            lua_getfield(L, -1, "color");
            auto color = luaL_checkstring(L, -1); lua_pop(L, 1);
            obj->SetColor(CColor::Parse(color));
            lua_getfield(L, -1, "radius");
            auto radius = luaL_checknumber(L, -1); lua_pop(L, 1);
            obj->SetRadius((FLOAT)radius);
            lua_getfield(L, -1, "fill");
            auto fill = luaL_checknumber(L, -1) != 0; lua_pop(L, 1);
            obj->SetFill(fill);
        }
    }
    break;
    case QRImage:
    {
        auto obj = static_cast<QRImageElement*>(o.get());
        {
            lua_getfield(L, -1, "color");
            auto color = luaL_checkstring(L, -1); lua_pop(L, 1);
            obj->SetColor(CColor::Parse(color));
            lua_getfield(L, -1, "text");
            auto text = luaL_checkstring(L, -1); lua_pop(L, 1);
            obj->SetText(text);
            lua_getfield(L, -1, "opacity");
            auto opacity = (FLOAT)luaL_checknumber(L, -1); lua_pop(L, 1);
            obj->SetOpacity(opacity);
        }
    }
    break;
    case Base64Image:
    {
        auto obj = static_cast<Base64ImageElement*>(o.get());
        {
            lua_getfield(L, -1, "text");
            auto text = luaL_checkstring(L, -1); lua_pop(L, 1);
            obj->SetText(text);
            lua_getfield(L, -1, "url");
            auto url = luaL_checkstring(L, -1); lua_pop(L, 1);
            obj->SetUrl(url);
            lua_getfield(L, -1, "opacity");
            auto opacity = (FLOAT)luaL_checknumber(L, -1); lua_pop(L, 1);
            obj->SetOpacity(opacity);
        }
    }
    break;
    case WireworldAutomaton:
    {
        auto obj = static_cast<WireworldAutomatonImageElement*>(o.get());
        {
            lua_getfield(L, -1, "text");
            auto text = luaL_checkstring(L, -1); lua_pop(L, 1);
            obj->SetText(text);
            lua_getfield(L, -1, "opacity");
            auto opacity = (FLOAT)luaL_checknumber(L, -1); lua_pop(L, 1);
            obj->SetOpacity(opacity);
        }
    }
    break;
    case Edit:
    {
        auto obj = static_cast<EditElement*>(o.get());
        lua_getfield(L, -1, "text");
        auto text = luaL_checkstring(L, -1); lua_pop(L, 1);
        obj->SetText(CString(text));
        lua_getfield(L, -1, "multiline");
        auto multiline = (cint)luaL_checkinteger(L, -1) == 0; lua_pop(L, 1);
        obj->SetMultiline(multiline);
        {
            Font font;
            lua_getfield(L, -1, "size");
            font.size = cint(luaL_checkinteger(L, -1)); lua_pop(L, 1);
            lua_getfield(L, -1, "family");
            font.fontFamily = CString(luaL_checkstring(L, -1)); lua_pop(L, 1);
            lua_getfield(L, -1, "bold");
            font.bold = luaL_checkinteger(L, -1) != 0; lua_pop(L, 1);
            lua_getfield(L, -1, "italic");
            font.italic = luaL_checkinteger(L, -1) != 0; lua_pop(L, 1);
            lua_getfield(L, -1, "underline");
            font.underline = luaL_checkinteger(L, -1) != 0; lua_pop(L, 1);
            obj->SetFont(font);
        }
    }
        break;
    default:
        break;
    }
    return 0;
}

int ui_refresh_obj(lua_State *L)
{
    auto arg = (cint)luaL_checkinteger(L, 2); lua_pop(L, 1);
    luaL_checktype(L, -1, LUA_TTABLE);
    lua_getfield(L, -1, "type");
    auto type = ElementId(cint(luaL_checkinteger(L, -1))); lua_pop(L, 1);
    lua_getfield(L, -1, "handle");
    auto handle = cint(luaL_checkinteger(L, -1)); lua_pop(L, 1);
    if (window->mapEle.find(handle) == window->mapEle.end())
        return luaL_argerror(L, 1, "Invalid obj id");
    auto o = window->mapEle[handle].lock();
    if (o->GetTypeId() != type)
        return luaL_argerror(L, 1, "Invalid obj type");
    switch (type)
    {
    case Empty:
        break;
    case SolidBackground:
        break;
    case SolidLabel:
        break;
    case GradientBackground:
        break;
    case RoundBorder:
        break;
    case QRImage:
        break;
    case Base64Image:
        break;
    case WireworldAutomaton:
    {
        auto obj = static_cast<WireworldAutomatonImageElement*>(o.get());
        {
            obj->Refresh(arg);
        }
    }
    break;
    case Edit:
        break;
    default:
        break;
    }
    return 0;
}

int ui_info(lua_State *L)
{
    lua_newtable(L);
    auto size = window->GetClientWindowSize();
    lua_pushstring(L, "width");
    lua_pushinteger(L, size.cx);
    lua_settable(L, -3);
    lua_pushstring(L, "height");
    lua_pushinteger(L, size.cy);
    lua_settable(L, -3);
    return 1;
}

int ui_win_set_minsize(lua_State* L)
{
    auto w = cint(luaL_checkinteger(L, 1));
    auto h = cint(luaL_checkinteger(L, 2));
    window->SetMinSize(CSize(w, h));
    return 0;
}

int ui_paint(lua_State *L)
{
    window->Redraw();
    return 0;
}

int ui_set_timer(lua_State *L)
{
    auto id = cint(luaL_checkinteger(L, 1));
    auto elapse = cint(luaL_checkinteger(L, 2));
    window->SetTimer(id, elapse);
    return 0;
}

int ui_kill_timer(lua_State *L)
{
    auto id = cint(luaL_checkinteger(L, 1));
    window->KillTimer(id);
    return 0;
}

int ui_quit(lua_State* L)
{
    auto code = cint(luaL_checkinteger(L, 1));
    PostQuitMessage(code);
    return 0;
}

int ui_play_song(lua_State *L)
{
    auto data = luaL_checkstring(L, 1);
    auto bin = base64_decode(data);
    DWORD dw = MAKELONG(MAKEWORD(bin[0], bin[1]), MAKEWORD(bin[2], bin[3]));
    auto b = (std::vector<byte>*)dw;
    auto &zplay = window->zplay;
    if (zplay)
    {
        zplay->Stop();
        zplay->Release();
        delete window->zplaydata;
    }
    zplay = libZPlay::CreateZPlay();
    window->zplaydata = b;
    auto binary = window->zplaydata->data();
    auto binsize = b->size();
    auto result = zplay->OpenStream(0, 0, binary, binsize, libZPlay::sfMp3);
    if (result == 0)
    {
        ATLTRACE(atlTraceWindowing, 0, "libzplay error: %s\n", zplay->GetError());
        delete window->zplaydata;
        window->zplaydata = nullptr;
        zplay->Release();
        zplay = nullptr;
    }
    zplay->Play();
    return 0;
}

int ui_music_ctl(lua_State *L)
{
    auto &zplay = window->zplay;
    auto ctlid = (cint)luaL_checkinteger(L, 1);
    switch (ctlid)
    {
    case 10:
    {
        if (!zplay)
            return 0;
        libZPlay::TStreamStatus status;
        zplay->GetStatus(&status);
        if (status.fPause == 1)
        {
            zplay->Resume();
        }
        else
        {
            if (status.fPlay == 0)
                zplay->Play();
            else
                zplay->Pause();
        }
    }
    break;
    case 11:
    {
        if (!zplay)
        {
            lua_pushstring(L, "播放进度");
            return 1;
        }
        libZPlay::TStreamTime pos;
        zplay->GetPosition(&pos);
        CStringA str;
        str.Format("%02u:%02u:%02u", pos.hms.hour, pos.hms.minute, pos.hms.second);
        lua_pushstring(L, str.GetBuffer(0));
        return 1;
    }
    break;
    case 12:
    {
        if (!zplay)
        {
            lua_pushstring(L, "歌曲信息");
            return 1;
        }
        libZPlay::TStreamInfo info;
        zplay->GetStreamInfo(&info);
        CStringA str;
        str.Format("长度：%02u:%02u:%02u.%03u\n码率：%dkbps",
            info.Length.hms.hour,
            info.Length.hms.minute,
            info.Length.hms.second,
            info.Length.hms.millisecond,
            info.Bitrate);
        lua_pushstring(L, str.GetBuffer(0));
        return 1;
    }
    break;
    case 13:
    {
        if (!zplay)
        {
            lua_pushstring(L, "未播放");
            return 1;
        }
        libZPlay::TStreamStatus status;
        zplay->GetStatus(&status);
        CStringA str;
        if (status.fPause == 1)
        {
            str = "暂停";
        }
        else
        {
            if (status.fPlay == 0)
                str = "停止";
            else
                str = "播放中";
        }
        lua_pushstring(L, str.GetBuffer(0));
        return 1;
    }
    break;
    case 14:
    {
        if (!zplay)
        {
            lua_pushinteger(L, -1);
            return 1;
        }
        libZPlay::TStreamTime pos;
        zplay->GetPosition(&pos);
        int sec = pos.hms.hour * 3600
            + pos.hms.minute * 60
            + pos.hms.second;
        lua_pushinteger(L, sec);
        return 1;
    }
    break;
    case 15:
    {
        if (!zplay)
        {
            lua_pushboolean(L, false);
            return 1;
        }
        libZPlay::TStreamStatus status;
        zplay->GetStatus(&status);
        CStringA str;
        if (status.fPause == 0 && status.fPlay == 0)
        {
            lua_pushboolean(L, false);
        }
        else
        {
            lua_pushboolean(L, true);
        }
        return 1;
    }
    break;
    case 16:
    {
        if (!zplay)
        {
            lua_pushinteger(L, 0);
            return 1;
        }
        unsigned l, r;
        zplay->GetPlayerVolume(&l, &r);
        lua_pushinteger(L, (l + r) / 2);
        return 1;
    }
    break;
    case 17:
    {
        if (!zplay)
        {
            return 0;
        }
        auto vol = (cint)luaL_checkinteger(L, 2);
        zplay->SetPlayerVolume(vol, vol);
        return 0;
    }
    break;
    case 18:
    {
        if (!zplay)
        {
            lua_pushnumber(L, 0);
            return 1;
        }
        libZPlay::TStreamTime pos;
        zplay->GetPosition(&pos);
        auto sec = pos.hms.hour * 3600
            + pos.hms.minute * 60
            + pos.hms.second;
        libZPlay::TStreamInfo info;
        zplay->GetStreamInfo(&info);
        auto full = info.Length.hms.hour * 3600
            + info.Length.hms.minute * 60
            + info.Length.hms.second;
        lua_pushnumber(L, 1.0 * sec / full);
        return 1;
    }
    break;
    case 20:
    {
        if (!zplay)
        {
            return 0;
        }
        libZPlay::TStreamStatus status;
        zplay->GetStatus(&status);
        CStringA str;
        if (status.fPause == 0 && status.fPlay == 0)
        {
            zplay->Play();
        }
        return 0;
    }
    break;
    }
    return 0;
}

int ui_parse_lyric(lua_State *L)
{
    std::string str = luaL_checkstring(L, 1);
    std::regex e(R"(\[\d*:\d*[.:]\d*\].*)");
    std::regex rep(R"(\[\d*:\d*[.:]\d*\](.*))");
    std::regex t(R"(\[(\d*):(\d*)([.:])(\d*)\].*)");
    std::map<int, std::string> ly;
    std::unordered_set<int> emptys;
    auto lyrics = std::split(str);
    for (auto lyr : lyrics)
    {
        std::smatch sm;
        if (std::regex_match(lyr, sm, e))
        {
            auto lycn = std::regex_replace(lyr, rep, "$1");
            for (std::string m : sm)
            {
                std::smatch tm;
                if (std::regex_match(m, tm, t))
                {
                    auto hour = atoi(tm[1].str().c_str());
                    auto minute = atoi(tm[2].str().c_str());
                    auto second = atoi(tm[4].str().c_str());
                    auto delim = tm[3].str();
                    if (delim == ".")
                        second = minute + 60 * hour;
                    else
                        second += 60 * minute + 3600 * hour;
                    if (lycn.empty())
                    {
                        emptys.insert(second);
                    }
                    else
                    {
                        ly.insert(std::make_pair(second, lycn));
                    }
                }
            }
        }
    }
    for (auto& em : emptys)
    {
        if (ly.find(em) == ly.end())
            ly.insert(std::make_pair(em, ""));
    }
    lua_newtable(L);
    for (auto& y : ly)
    {
        lua_pushinteger(L, y.first);
        lua_pushstring(L, y.second.c_str());
        lua_settable(L, -3);
    }
    return 1;
}