//
// Project: clibparser
// Created by bajdcc
//

#ifndef CLIBPARSER_CMEM_H
#define CLIBPARSER_CMEM_H

#include <vector>
#include <map>
#include "types.h"

#define MAX_PAGE_PER_PROCESS 65536

namespace clib {

    class imem {
    public:
        virtual void map_page(uint32_t addr, uint32_t id) = 0;
    };

    class cmem {
    public:
        explicit cmem(imem* m);

        cmem(const cmem&) = delete;
        cmem& operator=(const cmem&) = delete;

        uint32_t alloc(uint32_t size);
        uint32_t free(uint32_t addr);
        int page_size() const;
        int available() const;

        void copy_from(const cmem& mem);
        void dump_str(std::ostream& os) const;

    private:
        uint32_t new_page(uint32_t size);
        uint32_t new_page_single();
        uint32_t new_page_all(uint32_t size);

        void error(const string_t&) const;
        void dump() const;
        void check() const;

    private:
        std::vector<std::vector<byte>> memory;
        std::vector<uint32_t> memory_page;
        std::map<uint32_t, uint32_t> memory_free;
        std::map<uint32_t, uint32_t> memory_used;
        size_t available_size{ 0 };
        imem* m{ nullptr };
    };
}

#endif //CLIBPARSER_CMEM_H
