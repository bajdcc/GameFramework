//
// Project: clibparser
// Created by bajdcc
//

#ifndef CLIBPARSER_FS_H
#define CLIBPARSER_FS_H

#include "../CCGameFramework/base/parser2d/cvfs.h"

namespace clib {

    class vfs_node_text : public vfs_node_dec {
        friend class ext_web;
        friend class cextfs;
    public:
        bool available() const override;
        int index() const override;
        int get_length() const override;
        bool get_data(std::vector<byte>& data) const override;
    private:
        explicit vfs_node_text(const vfs_mod_query*, const string_t& str);
        std::string cache;
    };

    class vfs_node_stream_ext : public vfs_node_dec {
        friend class ext_web;
    public:
        bool available() const override;
        int index() const override;
        void advance() override;
        int write(byte c) override;
        int truncate() override;
        explicit vfs_node_stream_ext(const vfs_mod_query*, vfs_stream_t, vfs_stream_call*, const string_t& path);
        ~vfs_node_stream_ext();

        static vfs_node_dec* create(const vfs_mod_query* mod, vfs_stream_t s, vfs_stream_call* call, const string_t& path);

        bool is_success() const;

    private:
        bool success{ false };
        vfs_stream_t stream{ fss_none };
        vfs_stream_call* call{ nullptr };
    };

    class vfs_node_file : public vfs_node_dec {
        friend class cextfs;
    public:
        ~vfs_node_file() override;
        bool available() const override;
        int index() const override;
        int write(byte c) override;
        int truncate() override;
        void add_handle(int handle, vfs_op_t type) override;
        vfs_op_t get_handle(int handle, vfs_op_t type) override;
        void remove_handle(int handle) override;
        bool set_data(const std::vector<byte>& data) override;
        bool get_data(std::vector<byte>& data) const override;
        int get_length() const override;
    protected:
        explicit vfs_node_file(const vfs_mod_query*, const vfs_node::ref& ref);
        vfs_node::weak_ref node;
        int this_handle{ -1 };
    };

    class cextfs : public vfs_mod_query, public vfs_oper {
    public:
        cextfs();

        void reset();
        string_t get_user() const;
        string_t get_pwd() const;
        int get(const string_t& path, vfs_node_dec** dec = nullptr, vfs_func_t* f = nullptr) const;
        bool exist_vfs(const string_t& path) const;
        bool read_vfs(const string_t& path, std::vector<byte>& data) const;
        bool write_vfs(const string_t& path, const std::vector<byte>& data);
        bool write_text(const string_t& path, const string_t& data);

        void as_root(bool flag);
        void as_user(int uid, bool flag);

        int mkdir(const string_t& path) override;
        int touch(const string_t& path) override;
        int func(const string_t& path, vfs_func_t* f);
        int magic(const string_t& path, vfs_func_t* f, vfs_stream_t magic);
        int rm(const string_t& path) override;
        int rm_safe(const string_t& path) override;

        static void split_path(const string_t& path, std::vector<string_t>& args, char c);
        static string_t get_filename(const string_t& path);

        uint64 size() const;

    private:
        vfs_node::ref new_node(vfs_file_t type);
        vfs_node::ref get_node(const string_t& path) const;
        int _mkdir(const string_t& path, vfs_node::ref& cur);
        void _touch(vfs_node::ref& node);

        string_t combine(const string_t& pwd, const string_t& path) const;
        int macro(const std::vector<string_t>& m, const vfs_node::ref& node, vfs_node_dec** dec) const;
        int macrofile(const std::vector<string_t>& m, const vfs_node::ref& node, vfs_node_dec** dec) const;
        string_t trans(const string_t& path) const;
        void ll(const string_t& name, const vfs_node::ref& node, std::ostream& os) const;

        void error(const string_t&);

        static time_t now();

        char* file_time(const time_t& t) const;
        bool can_rm(const vfs_node::ref& node) const;

    public:
        bool can_mod(const vfs_node::ref& node, int mod) const override;

        enum user_name {
            user = 1,
            ext = 10,
        };
    private:
        std::map<int, vfs_user> account;
        std::shared_ptr<vfs_node> root;
        int current_user{ 0 };
        int last_user{ 0 };
        string_t pwd;
        int year{ 0 };
    };
}

#endif //CLIBPARSER_FS_H
