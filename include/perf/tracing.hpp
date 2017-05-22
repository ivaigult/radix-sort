#pragma once

#if defined(ITT_NOTIFY_FOUND)
#  include <ittnotify>
#endif

namespace perf {
#if defined(ITT_NOTIFY_FOUND)
namespace detail {
    __itt_domain* get_domain() {
        static __itt_domain* s_domain = __itt_domain_createA("perf");
        return s_domain;
    }
}

struct task {
    task(const char* msg) 
        : _domain(detail::get_domain())
    {
        __itt_string_handle* string_hndl = __itt_string_handle_create(msg);
        __itt_task_begin(_domain, __itt_null, __itt_null, string_hndl);
    }
    ~task() {
        __itt_task_end(_domain);
    }
private:
    __itt_domain* _domain;
};

#else
    struct task {
        task(const char* /*msg*/) {}
        ~task() {}
    };
#endif
}