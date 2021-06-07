#pragma once

namespace exy {
struct Identifiers {
    void initialize();
    void dispose();

    Identifier get(const char *v, int vlen, UINT vhash);
    Identifier get(const char *v, int vlen);
    Identifier get(const char *v, const char *vend);
    Identifier get(const String&);
    Identifier get(Identifier);

    Identifier random(const char *prefix, int prefixlen);
private:
    Mem              mem{};
    Dict<Identifier> list{};
    SRWLOCK          srw{};
    int              randomCounter = 1000;

    void lock() { AcquireSRWLockExclusive(&srw); }
    void unlock() { ReleaseSRWLockExclusive(&srw); }
    Identifier append(const char *v, int vlen, UINT vhash);
};

__declspec(selectany) Identifiers ids;
} // namespace exy