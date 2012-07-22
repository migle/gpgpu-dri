#pragma once

#include <iostream>
#include <limits>

template < typename T >
class hex_dump {
public:
    hex_dump(T const* ptr, std::size_t n, std::size_t c = 1)
        : base(ptr), size(n), cols(c) {}

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
                os.width(6),
                os << i << ':';
            }
            os << '\t';
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
};
