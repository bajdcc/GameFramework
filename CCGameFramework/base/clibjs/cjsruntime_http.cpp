//
// Project: clibjs
// Created by bajdcc
//

#include "stdafx.h"
#include "cjsruntime.h"
#include "cjsgui.h"

#define MAX_HTTP_TASKS 6

namespace clib {

    static cjsruntime::resp_t js_http_request(cjsruntime::http_struct_simple* hs) {
        using namespace clib;
        cjsruntime::resp_t r;
        auto conn = new RestClient::Connection("");
        conn->SetTimeout(5);
        conn->SetUserAgent("Mozilla/5.0 (Windows NT 6.3; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/78.0.3904.108 Safari/537.36");
        conn->FollowRedirects(true);
        RestClient::HeaderFields headers;
        headers["Accept"] = "*/*";
        for (const auto& s : hs->headers) {
            headers[std::get<0>(s)] = std::get<1>(s);
        }
        conn->SetHeaders(headers);
        switch (hs->method_type) {
        case cjsruntime::M_GET:
            r.response = conn->get(hs->url);
            break;
        case cjsruntime::M_POST:
            conn->AppendHeader("Content-Type", "application/json");
            r.response = conn->post(hs->url, hs->payload);
            break;
        case cjsruntime::M_PUT:
            conn->AppendHeader("Content-Type", "application/json");
            r.response = conn->put(hs->url, hs->payload);
            break;
        case cjsruntime::M_PATCH:
            conn->AppendHeader("Content-Type", "application/json");
            r.response = conn->patch(hs->url, hs->payload);
            break;
        case cjsruntime::M_DELETE:
            r.response = conn->del(hs->url);
            break;
        case cjsruntime::M_HEAD:
            r.response = conn->head(hs->url);
            break;
        case cjsruntime::M_OPTIONS:
            r.response = conn->options(hs->url);
            break;
        default:
            r.response = conn->get(hs->url);
            break;
        }
        r.info = conn->GetInfo2();
        auto f = r.response.headers.find("Content-Type");
        if (f != r.response.headers.end()) {
            std::regex re(R"(charset=utf-8)", std::regex::ECMAScript | std::regex::icase);
            if (std::regex_search(f->second, re)) {
                cjsruntime::convert_utf8_to_gbk(r.response.body);
            }
            std::regex re2(R"(^(text/.+|application/json))", std::regex::ECMAScript | std::regex::icase);
            if (!std::regex_search(f->second, re2)) {
                r.binary = true;
            }
        }
        delete hs;
        return r;
    }

    void cjsruntime::eval_http() {
        if (current_stack || !stack.empty())
            return;
        using namespace std::chrono_literals;
        auto& i = global_http.running_tasks;
        for (const auto& s : global_http.caches) {
            if (i >= MAX_HTTP_TASKS)
                break;
            if (s->state == 0) {
                s->state = 1;
                auto arg = new http_struct_simple();
                arg->url = s->url;
                arg->method = s->method;
                arg->method_type = s->method_type;
                arg->headers = s->headers;
                arg->payload = s->payload;
                CString str;
                str.Format(L"HTTP Query：'%S'", s->url.c_str());
                cjsgui::singleton().add_stat(str);
                s->fut = std::async(js_http_request, arg);
                i++;
            }
        }
        if (i > 0) {
            auto j = i;
            auto t = 0s;
            for (auto k = global_http.caches.begin(); k != global_http.caches.end(); k++) {
                if (j <= 0)
                    break;
                auto& s = *k;
                if (s->state == 1) {
                    if (s->fut.wait_for(t) == std::future_status::ready) {
                        i--;
                        s->resp = s->fut.get();
                        s->state = 2;
                        auto& callback = s->callback;
                        stack.clear();
                        stack.push_back(permanents.default_stack);
                        current_stack = stack.back();
                        current_stack->name = callback->name;
                        current_stack->exec = callback->name;
                        current_stack->envs = permanents.global_env;
                        current_stack->_this = permanents.global_env;
                        current_stack->trys.push_back(std::make_shared<sym_try_t>(sym_try_t{}));
                        paths.emplace_back(ROOT_DIR);
                        js_value::weak_ref env = stack.front()->envs;
                        CString str;
                        str.Format(L"HTTP Success：'%S', Code: %d", s->url.c_str(), s->resp.response.code);
                        cjsgui::singleton().add_stat(str);
                        std::vector<js_value::weak_ref> args;
                        auto obj = new_object();
                        obj->add("result", new_string("success"));
                        obj->add("code", new_number(s->resp.response.code));
                        if (s->resp.binary) {
                            auto buf = new_buffer();
                            std::vector<char> v(s->resp.response.body.begin(), s->resp.response.body.end());
                            buf->set_buffer(*this, v);
                            obj->add("body", buf);
                        }
                        else
                            obj->add("body", new_string(s->resp.response.body));
                        obj->add("binary", new_boolean(s->resp.binary));
                        auto header = new_object();
                        for (const auto& j : s->resp.response.headers) {
                            header->add(j.first, new_string(j.second));
                        }
                        obj->add("headers", header);
                        auto time = new_object();
                        time->add("totalTime", new_number(s->resp.info.totalTime));
                        time->add("nameLookupTime", new_number(s->resp.info.nameLookupTime));
                        time->add("connectTime", new_number(s->resp.info.connectTime));
                        time->add("appConnectTime", new_number(s->resp.info.appConnectTime));
                        time->add("preTransferTime", new_number(s->resp.info.preTransferTime));
                        time->add("startTransferTime", new_number(s->resp.info.startTransferTime));
                        time->add("redirectTime", new_number(s->resp.info.redirectTime));
                        obj->add("time", time);
                        args.push_back(obj);
                        call_api(callback, env, args, callback->attr | jsv_function::at_fast);
                        current_stack = stack.back();
                        k = global_http.caches.erase(k);
                        permanents.last = 3;
                        break;
                    }
                }
                j--;
            }
        }
    }
}