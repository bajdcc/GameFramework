//
// Project: clibparser
// Created by bajdcc
//

#ifndef CLIBPARSER_CVFS_H
#define CLIBPARSER_CVFS_H

#include <vector>
#include <map>
#include <memory>
#include "memory.h"

#define FILE_ROOT "./script/code"
#define WAIT_CHAR 0x10000
#define READ_EOF 0x2000
#define READ_ERROR (READ_EOF + 1)

namespace clib {

    // 权限
    struct vfs_mod {
        char rwx[3];
    };

    enum vfs_file_t {
        fs_file,
        fs_dir,
        fs_func,
        fs_magic,
    };

    enum vfs_stream_t {
        fss_none,
        fss_random,
        fss_null,
        fss_net,
        fss_console,
        fss_music,
    };

    class vfs_node_dec;
    class vfs_mod_query;
    class vfs_func_t {
    public:
        virtual string_t stream_callback(const string_t& path) = 0;
        virtual vfs_stream_t stream_type(const string_t& path) const = 0;
        virtual vfs_node_dec* stream_create(const vfs_mod_query* mod, vfs_stream_t type, const string_t& path) = 0;
    };

    // 结点
    struct vfs_node {
        using ref = std::shared_ptr<vfs_node>;
        using weak_ref = std::weak_ptr<vfs_node>;
        vfs_file_t type;
        vfs_mod mod[3];
        int owner;
        struct {
            time_t create;
            time_t access;
            time_t modify;
        } time;
        int refs;
        bool locked;
        string_t name;
        std::map<string_t, ref> children;
        std::vector<byte> data;
        vfs_func_t* callback;
        vfs_stream_t magic;
        weak_ref parent;
    };

    struct vfs_user {
        int id;
        string_t name;
        string_t password;
    };

    class vfs_mod_query {
    public:
        virtual bool can_mod(const vfs_node::ref& node, int mod) const = 0;
    };
    class vfs_node_dec {
    public:
        virtual bool available() const = 0;
        virtual int index() const = 0;
        virtual void advance();
        virtual int write(byte c);
        virtual int truncate();
        virtual ~vfs_node_dec() = default;
    protected:
        explicit vfs_node_dec(const vfs_mod_query*);
        uint idx{ 0 };
        const vfs_mod_query* mod{ nullptr };
    };

    class vfs_node_solid : public vfs_node_dec {
        friend class cvfs;
    public:
        ~vfs_node_solid() override;
        bool available() const override;
        int index() const override;
        int write(byte c) override;
        int truncate() override;
    private:
        explicit vfs_node_solid(const vfs_mod_query*, const vfs_node::ref& ref);
        vfs_node::weak_ref node;
    };

    class vfs_node_cached : public vfs_node_dec {
        friend class cvfs;
    public:
        bool available() const override;
        int index() const override;
    private:
        explicit vfs_node_cached(const vfs_mod_query*, const string_t& str);
        string_t cache;
    };

    class vfs_stream_call {
    public:
        virtual int stream_index(vfs_stream_t type) = 0;
        virtual string_t stream_net(vfs_stream_t type, const string_t& path) = 0;
        virtual int stream_write(vfs_stream_t type, byte c) = 0;
        virtual string_t stream_path(const string_t& path) = 0;
    };

    class vfs_node_stream : public vfs_node_dec {
        friend class cvfs;
    public:
        bool available() const override;
        int index() const override;
        void advance() override;
        int write(byte c) override;
        int truncate() override;
        explicit vfs_node_stream(const vfs_mod_query*, vfs_stream_t, vfs_stream_call*);

        static vfs_node_dec* create(const vfs_mod_query* mod, vfs_stream_t s, vfs_stream_call* call);
    private:
        vfs_stream_t stream{ fss_none };
        vfs_stream_call* call{ nullptr };
    };

    class vfs_node_stream_write : public vfs_node_dec {
        friend class cvfs;
    public:
        bool available() const override;
        int index() const override;
        void advance() override;
        int write(byte c) override;
        int truncate() override;
        explicit vfs_node_stream_write(const vfs_mod_query*, vfs_stream_t, vfs_stream_call*);

        static vfs_node_dec* create(const vfs_mod_query* mod, vfs_stream_t s, vfs_stream_call* call);
    private:
        vfs_stream_t stream{ fss_none };
        vfs_stream_call* call{ nullptr };
    };

    class cvfs : public vfs_mod_query {
    public:
        cvfs();

        void reset();
        string_t get_user() const;
        string_t get_pwd() const;
        int get(const string_t& path, vfs_node_dec** dec = nullptr, vfs_func_t* f = nullptr) const;
        bool read_vfs(const string_t& path, std::vector<byte>& data) const;
        bool write_vfs(const string_t& path, const std::vector<byte>& data);

        void as_root(bool flag);

        int cd(const string_t& path);
        int mkdir(const string_t& path);
        int touch(const string_t& path);
        int func(const string_t& path, vfs_func_t* f);
        int magic(const string_t& path, vfs_func_t* f, vfs_stream_t magic);
        int rm(const string_t& path);
        int rm_safe(const string_t& path);
        void load(const string_t& path);

        static void split_path(const string_t& path, std::vector<string_t>& args, char c);
        static string_t get_filename(const string_t& path);
        string_t get_realpath(const string_t& path);

    private:
        vfs_node::ref new_node(vfs_file_t type);
        vfs_node::ref get_node(const string_t& path) const;
        int _mkdir(const string_t& path, vfs_node::ref& cur);
        void _touch(vfs_node::ref& node);

        string_t combine(const string_t& pwd, const string_t& path) const;
        int macro(const std::vector<string_t>& m, const vfs_node::ref& node, vfs_node_dec** dec) const;
        void ll(const string_t& name, const vfs_node::ref& node, std::ostream& os) const;

        void error(const string_t&);

        static time_t now();

        char* file_time(const time_t& t) const;
        bool can_rm(const vfs_node::ref& node) const;

    public:
        bool can_mod(const vfs_node::ref& node, int mod) const override;

    private:
        std::vector<vfs_user> account;
        std::shared_ptr<vfs_node> root;
        int current_user{ 0 };
        int last_user{ 0 };
        string_t pwd;
        int year{ 0 };
    };
}

#endif //CLIBPARSER_CVFS_H
