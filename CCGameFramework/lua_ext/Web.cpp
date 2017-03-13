#include "stdafx.h"
#include "ui/window/Window.h"
#include "Web.h"
#include "ext.h"
#include <curl/curl.h>
#include "base64/b64.h"
#include <base/utils.h>

static const luaL_Reg ui_lib[] = {
    { "get", web_http_get },
    { "getb", web_http_get_b64 },
    { "post", web_http_post },
    { "postb", web_http_post_b64 },
    { nullptr, nullptr }
};

static int luaopen_web(lua_State *L) {
    luaL_newlib(L, ui_lib);
    return 1;
}

void lua_ext_register_web(lua_State *L)
{
    luaL_requiref(L, "Web", luaopen_web, 1);
}

struct web_http_request
{
    cint id;
    std::string url;
    bool b64;
    bool post;
    std::string postfield;
};

struct web_http_response
{
    cint id;
    UINT code;
    std::string text;
    bool b64;
    bool post;
};

static size_t http_get_process(void *data, size_t size, size_t nmemb, std::string &content)
{
    auto sizes = size * nmemb;
    content += std::string((char*)data, sizes);
    return sizes;
}

static size_t http_get_process_bin(void *data, size_t size, size_t nmemb, std::vector<byte> &content)
{
    auto sizes = size * nmemb;
    auto bin = (byte*)data;
    for (size_t i = 0; i < sizes; ++i)
    {
        content.push_back(bin[i]);
    }
    return sizes;
}

void pass_event(evutil_socket_t fd, short event, void *arg)
{
    auto response = (web_http_response*)arg;
    auto L = window->get_state();
    lua_getglobal(L, "PassEventToScene");
    if (!response->post)
        lua_pushinteger(L, WE_HttpGet);
    else
        lua_pushinteger(L, WE_HttpPost);
    lua_pushinteger(L, response->id);
    lua_pushinteger(L, response->code);
    lua_pushstring(L, response->text.c_str());
    lua_call(L, 4, 0);
    delete response;
}

CString Utf8ToStringT(LPCSTR str)
{
    _ASSERT(str);
    USES_CONVERSION;
    auto length = MultiByteToWideChar(CP_UTF8, 0, str, -1, nullptr, 0);
    CString s;
    auto buf = s.GetBuffer(length + 1);
    ZeroMemory(buf, (length + 1) * sizeof(WCHAR));
    MultiByteToWideChar(CP_UTF8, 0, str, -1, buf, length);
    s.ReleaseBuffer();
    return s;
}

CStringA StringTToUtf8(CString str)
{
    USES_CONVERSION;
    auto length = WideCharToMultiByte(CP_UTF8, 0, str, -1, nullptr, 0, nullptr, nullptr);
    CStringA s;
    auto buf = s.GetBuffer(length + 1);
    ZeroMemory(buf, (length + 1) * sizeof(CHAR));
    WideCharToMultiByte(CP_UTF8, 0, str, -1, buf, length, nullptr, nullptr);
    return s;
}

void http_thread(web_http_request *request)
{
    auto url = request->url;
    auto response = new web_http_response();
    auto post = request->post;
    auto postfield = request->postfield;
    response->id = request->id;
    response->b64 = request->b64;
    response->post = request->post;
    auto curl = curl_easy_init();
    delete request;
    request = nullptr;
    if (curl) {
        auto params = std::split(postfield, '&');
        postfield.clear();
        for (auto& p : params)
        {
            auto p2 = std::split(p, '=');
            if (p2.size() == 2)
            {
                postfield += p2[0];
                postfield += '=';
                auto utf8 = StringTToUtf8(CString(p2[1].c_str()));
                auto pf = curl_easy_escape(curl, utf8.GetBuffer(0), utf8.GetLength());
                postfield += pf;
                curl_free(pf);
                postfield += '&';
            }
        }
        if (!postfield.empty() && postfield.back() == '&')
        {
            postfield.pop_back();
        }
        params.clear();
        std::string text;
        auto bindata = new std::vector<byte>();
        bindata->reserve(102400);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/50.0.2661.102 Safari/537.36");
        if (post)
        {
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str());
        }
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 60L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, TRUE);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, TRUE);
        if (!response->b64)
        {
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &text);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &http_get_process);
        }
        else
        {
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, bindata);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &http_get_process_bin);
        }
        auto res = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response->code);
        if (res == CURLE_OK)
        {
            char *content_type;
            curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &content_type);
            auto ct = CStringA(content_type);
            if (response->b64)
            {
                std::vector<byte> b;
                DWORD dw = (DWORD)bindata;
                b.push_back(LOBYTE(LOWORD(dw)));
                b.push_back(HIBYTE(LOWORD(dw)));
                b.push_back(LOBYTE(HIWORD(dw)));
                b.push_back(HIBYTE(HIWORD(dw)));
                response->text = base64_encode(b);
            }
            else
            {
                if (ct.Find("UTF-8"))
                    //TODO: º«Óï»áÏÔÊ¾ÂÒÂë
                    response->text = CStringA(Utf8ToStringT(text.c_str()));
                else
                    response->text = text.c_str();
            }
            auto ev = window->get_event();
            struct timeval tv;
            auto evt = evtimer_new(ev, &pass_event, response);
            evutil_timerclear(&tv);
            tv.tv_sec = 0;
            tv.tv_usec = 100;
            evtimer_add(evt, &tv);
        }
        curl_easy_cleanup(curl);
    }
}

void start_thread(evutil_socket_t fd, short event, void *arg)
{
    std::thread th(http_thread, (web_http_request*)arg);
    th.detach();
}

int web_http_get_internal(lua_State* L, bool b64 = false, bool post = false, std::string postfield = "")
{
    auto request = new web_http_request();
    request->b64 = b64;
    request->url = luaL_checkstring(L, 1);
    request->id = (cint)luaL_checkinteger(L, 2);
    request->post = post;
    if (post)
    {
        request->postfield = luaL_checkstring(L, 3);
    }
    auto ev = window->get_event();
    struct timeval tv;
    auto evt = evtimer_new(ev, &start_thread, request);
    evutil_timerclear(&tv);
    tv.tv_sec = 0;
    tv.tv_usec = 100;
    evtimer_add(evt, &tv);
    return 0;
}

int web_http_get(lua_State* L)
{
    return web_http_get_internal(L, false);
}

int web_http_get_b64(lua_State* L)
{
    return web_http_get_internal(L, true);
}

int web_http_post(lua_State* L)
{
    return web_http_get_internal(L, false, true);
}

int web_http_post_b64(lua_State* L)
{
    return web_http_get_internal(L, true, true);
}
