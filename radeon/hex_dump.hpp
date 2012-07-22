#pragma once

#include <iostream>
#include <limits>

template < typename T >
class hex_dump {
public:
    hex_dump(T const* ptr, std::size_t n, std::size_t c = 1, std::size_t a = 0)
        : base(ptr), size(n), cols(c), addr(a) {}

    friend std::ostream& operator << (std::ostream& os, hex_dump const& hd)
    {
        std::ios_base::fmtflags
            previous_flags = os.flags(std::ios_base::right | std::ios_base::hex);
        char previous_fill = os.fill('0');

        T const* p = hd.base;
        for (std::size_t i = 0; i != hd.size; ++i, ++p)
        {
            if (i % hd.cols == 0) {
                if (i != 0) os << '\n';
                if (hd.addr != 0) {
                    os.width(hd.addr);
                    os << i << ':' << '\t' << std::flush;
                }
            }
            else os << '\t';
            os.width(std::numeric_limits<T>::digits / 4);
            os << *p;
        }

        os.flags(previous_flags);
        os.fill(previous_fill);
        return os;
    }

private:
    T const* base;
    std::size_t size;
    std::size_t cols;
    std::size_t addr;
};
