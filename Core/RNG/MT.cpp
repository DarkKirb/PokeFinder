/*
 * This file is part of PokéFinder
 * Copyright (C) 2017-2020 by Admiral_Fish, bumba, and EzPzStreamz
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "MT.hpp"

MT::MT(u32 seed)
{
    mt[0] = seed;

    for (index = 1; index < 624; index++)
    {
        seed = 0x6c078965 * (seed ^ (seed >> 30)) + index;
        mt[index] = seed;
    }
}

void MT::advance(u32 advances)
{
    index += advances;
    while (index >= 624)
    {
        shuffle();
    }
}

u32 MT::next()
{
    if (index >= 624)
    {
        shuffle();
    }

    u32 y = mt[index++];
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680;
    y ^= (y << 15) & 0xefc60000;
    y ^= (y >> 18);

    return y;
}

u16 MT::nextUShort()
{
    return next() >> 16;
}

void MT::shuffle()
{
#ifdef SIMD_256BIT
    vuint32x4 upperMask = v32x4_splat(0x80000000);
    vuint32x4 lowerMask = v32x4_splat(0x7fffffff);
    vuint32x4 matrix = v32x4_splat(0x9908b0df);
    vuint32x4 one = v32x4_splat(1);

    vuint32x8 upperMask256 = v32x8_splat(0x80000000);
    vuint32x8 lowerMask256 = v32x8_splat(0x7fffffff);
    vuint32x8 matrix256 = v32x8_splat(0x9908b0df);
    vuint32x8 one256 = v32x8_splat(1);

    for (int i = 0; i < 224; i += 8)
    {
        vuint32x8 m0 = v32x8_load(&mt[i]);
        vuint32x8 m1 = v32x8_load(&mt[i + 1]);
        vuint32x8 m2 = v32x8_load(&mt[i + 397]);

        vuint32x8 y = v32x8_or(v32x8_and(m0, upperMask256), v32x8_and(m1, lowerMask256));
        vuint32x8 y1 = v32x8_shr(y, 1);
        vuint32x8 mag01 = v32x8_and(v32x8_cmpeq(v32x8_and(y, one256), one256), matrix256);

        v32x8_store(&mt[i], v32x8_xor(v32x8_xor(y1, mag01), m2));
    }

    vuint32x4 last = v32x4_insert(v32x4_load(&mt[621]), mt[0], 3);
    {
        vuint32x4 m0 = v32x4_load(&mt[224]);
        vuint32x4 m1 = v32x4_load(&mt[225]);

        vuint32x4 y = v32x4_or(v32x4_and(m0, upperMask), v32x4_and(m1, lowerMask));
        vuint32x4 y1 = v32x4_shr(y, 1);
        vuint32x4 mag01 = v32x4_and(v32x4_cmpeq(v32x4_and(y, one), one), matrix);

        v32x4_store(&mt[224], v32x4_xor(v32x4_xor(y1, mag01), last));
    }

    for (int i = 228; i < 620; i += 8)
    {
        vuint32x8 m0 = v32x8_load(&mt[i]);
        vuint32x8 m1 = v32x8_load(&mt[i + 1]);
        vuint32x8 m2 = v32x8_load(&mt[i - 227]);

        vuint32x8 y = v32x8_or(v32x8_and(m0, upperMask256), v32x8_and(m1, lowerMask256));
        vuint32x8 y1 = v32x8_shr(y, 1);
        vuint32x8 mag01 = v32x8_and(v32x8_cmpeq(v32x8_and(y, one256), one256), matrix256);

        v32x8_store(&mt[i], v32x8_xor(v32x8_xor(y1, mag01), m2));
    }

    {
        vuint32x4 m0 = v32x4_load(&mt[620]);
        vuint32x4 m2 = v32x4_load(&mt[393]);

        vuint32x4 y = v32x4_or(v32x4_and(m0, upperMask), v32x4_and(last, lowerMask));
        vuint32x4 y1 = v32x4_shr(y, 1);
        vuint32x4 mag01 = v32x4_and(v32x4_cmpeq(v32x4_and(y, one), one), matrix);

        v32x4_store(&mt[620], v32x4_xor(v32x4_xor(y1, mag01), m2));
    }
#else
    vuint32x4 upperMask = v32x4_splat(0x80000000);
    vuint32x4 lowerMask = v32x4_splat(0x7fffffff);
    vuint32x4 matrix = v32x4_splat(0x9908b0df);
    vuint32x4 one = v32x4_splat(1);

    for (int i = 0; i < 224; i += 4)
    {
        vuint32x4 m0 = v32x4_load(&mt[i]);
        vuint32x4 m1 = v32x4_load(&mt[i + 1]);
        vuint32x4 m2 = v32x4_load(&mt[i + 397]);

        vuint32x4 y = v32x4_or(v32x4_and(m0, upperMask), v32x4_and(m1, lowerMask));
        vuint32x4 y1 = v32x4_shr(y, 1);
        vuint32x4 mag01 = v32x4_and(v32x4_cmpeq(v32x4_and(y, one), one), matrix);

        v32x4_store(&mt[i], v32x4_xor(v32x4_xor(y1, mag01), m2));
    }

    vuint32x4 last = v32x4_insert(v32x4_load(&mt[621]), mt[0], 3);
    {
        vuint32x4 m0 = v32x4_load(&mt[224]);
        vuint32x4 m1 = v32x4_load(&mt[225]);

        vuint32x4 y = v32x4_or(v32x4_and(m0, upperMask), v32x4_and(m1, lowerMask));
        vuint32x4 y1 = v32x4_shr(y, 1);
        vuint32x4 mag01 = v32x4_and(v32x4_cmpeq(v32x4_and(y, one), one), matrix);

        v32x4_store(&mt[224], v32x4_xor(v32x4_xor(y1, mag01), last));
    }

    for (int i = 228; i < 620; i += 4)
    {
        vuint32x4 m0 = v32x4_load(&mt[i]);
        vuint32x4 m1 = v32x4_load(&mt[i + 1]);
        vuint32x4 m2 = v32x4_load(&mt[i - 227]);

        vuint32x4 y = v32x4_or(v32x4_and(m0, upperMask), v32x4_and(m1, lowerMask));
        vuint32x4 y1 = v32x4_shr(y, 1);
        vuint32x4 mag01 = v32x4_and(v32x4_cmpeq(v32x4_and(y, one), one), matrix);

        v32x4_store(&mt[i], v32x4_xor(v32x4_xor(y1, mag01), m2));
    }

    {
        vuint32x4 m0 = v32x4_load(&mt[620]);
        vuint32x4 m2 = v32x4_load(&mt[393]);

        vuint32x4 y = v32x4_or(v32x4_and(m0, upperMask), v32x4_and(last, lowerMask));
        vuint32x4 y1 = v32x4_shr(y, 1);
        vuint32x4 mag01 = v32x4_and(v32x4_cmpeq(v32x4_and(y, one), one), matrix);

        v32x4_store(&mt[620], v32x4_xor(v32x4_xor(y1, mag01), m2));
    }
#endif

    index -= 624;
}
