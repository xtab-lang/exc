#pragma once

namespace exy {
struct Identifiers {
    Identifier kw_main;
    Identifier kw_GET;
    Identifier kw_POST;
    Identifier kw_DELETE;
    Identifier kw_PUT;
    Identifier kw_HEAD;
    Identifier kw_CONNECT;
    Identifier kw_OPTIONS;
    Identifier kw_TRACE;
    Identifier kw_PATCH;

    void initialize();
    void dispose();

    Identifier get(const CHAR *v, INT vlen, UINT vhash);
    Identifier get(const CHAR *v, INT vlen);
    Identifier get(const CHAR *v, const CHAR *vend);
    Identifier get(const String&);
    Identifier get(Identifier);

    Identifier random(const CHAR *prefix, INT prefixlen);
private:
    Mem              mem{};
    Dict<Identifier> list{};
    SRWLOCK          srw{};
    INT              randomCounter = 1000;

    void lock() { AcquireSRWLockExclusive(&srw); }
    void unlock() { ReleaseSRWLockExclusive(&srw); }
    Identifier append(const CHAR *v, INT vlen, UINT vhash);
};

__declspec(selectany) Identifiers ids{};
} // namespace exy