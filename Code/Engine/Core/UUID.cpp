#include "Engine/Core/UUID.hpp"

#include <cstdlib>
#include <string>


// random UUID generation from Java
UUID UUID::randomUUID()
{
    // code for RAND_MAX = 0x7FFF;

    uint16_t randbit = (uint16_t) std::rand();
    uint16_t rands[8];
    for (auto& r : rands)
    {
        r = (uint16_t) std::rand();
        r |= randbit << 15;
        randbit >>= 1;
    }

    char* randomBytes = (char*) &rands[0];
    // char randomBytes[16];
    // for (char& c : randomBytes)
    //     c = std::rand() & 0xff;
    randomBytes[6] &= 0x0f;  /* clear version        */
    randomBytes[6] |= 0x40;  /* set to version 4     */
    randomBytes[8] &= 0x3f;  /* clear variant        */
    randomBytes[8] |= 0x80;  /* set to IETF variant  */
    return UUID(randomBytes);
}

UUID::UUID(const char* bits)
{
    long msb = 0;
    long lsb = 0;

    for (int i = 0; i < 8; i++)
        msb = (msb << 8) | (bits[i] & 0xff);
    for (int i = 8; i < 16; i++)
        lsb = (lsb << 8) | (bits[i] & 0xff);
    m_msb = msb;
    m_lsb = lsb;
}

constexpr UUID::UUID(long msb, long lsb)
    : m_msb(msb)
    , m_lsb(lsb)
{

}

UUID::UUID(const UUID& copyFrom)
    : m_msb(copyFrom.m_msb)
    , m_lsb(copyFrom.m_lsb)
{

}

bool operator<(const UUID& a, const UUID& b)
{
    return a.m_lsb < b.m_lsb && a.m_msb < b.m_msb;
}

UUID UUID::invalidUUID()
{
    return UUID(0, 0);
}
