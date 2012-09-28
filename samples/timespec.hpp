#pragma once

#include <sys/time.h>

inline double as_seconds(const timespec& ts)
    { return ts.tv_nsec * 1e-9 + ts.tv_sec; }
inline timespec& operator += (timespec& L, const timespec& R) {
    L.tv_sec += R.tv_sec;
    L.tv_nsec += R.tv_nsec;
    if (L.tv_nsec >= 1000000000) {
        L.tv_sec++;
        L.tv_nsec -= 1000000000;
    }
    return L;
}
inline timespec& operator -= (timespec& L, const timespec& R) {
    L.tv_sec -= R.tv_sec;
    L.tv_nsec -= R.tv_nsec;
    if (L.tv_nsec < 0) {
        L.tv_sec--;
        L.tv_nsec += 1000000000;
    }
    return L;
}
inline timespec operator + (const timespec& L, const timespec& R)
    { timespec T(L); return T += R; }
inline timespec operator - (const timespec& L, const timespec& R)
    { timespec T(L); return T -= R; }
inline bool operator == (const timespec& L, const timespec& R)
    { return L.tv_sec == R.tv_sec && L.tv_nsec == R.tv_nsec; }
inline bool operator != (const timespec& L, const timespec& R)
    { return !(L == R); }
inline bool operator < (const timespec& L, const timespec& R)
    { return L.tv_sec == R.tv_sec ? L.tv_nsec < R.tv_nsec : L.tv_sec < R.tv_sec; }
inline bool operator > (const timespec& L, const timespec& R)
    { return R < L; }
inline bool operator >= (const timespec& L, const timespec& R)
    { return !(L < R); }
inline bool operator <= (const timespec& L, const timespec& R)
    { return !(R < L); }
