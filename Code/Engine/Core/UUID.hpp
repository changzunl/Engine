#pragma once


class UUID
{
public:
    constexpr UUID(long msb, long lsb);
    UUID(const UUID& copyFrom);

    static UUID invalidUUID();
    static UUID randomUUID();

private:
    UUID(const char* bits);

public:
    inline long getMSB() { return m_msb; }
    inline long getLSB() { return m_lsb; }

    inline void operator=(const UUID& other) { m_msb = other.m_msb; m_lsb = other.m_lsb; }
    inline bool operator==(const UUID& other) { return m_msb == other.m_msb && m_lsb == other.m_lsb; }
    inline bool operator!=(const UUID& other) { return !operator==(other); }

    friend bool operator<(const UUID& a, const UUID& b);

private:
    long m_lsb; // least sig bits
    long m_msb; // most sig bits
};
