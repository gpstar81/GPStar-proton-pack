/**
 *   ShuffleMusic - Music shuffling functionality for GPStar devices.
 *   Provides structure and functions for lightweight music shuffling.
 *   Copyright (C) 2023-2026 Michael Rajotte, Dustin Grau, Nomake Wan
 *
 *   This code was adapted from "Stateless Shuffling" by Alan Wolfe.
 *   https://github.com/SEED-EA/O1ShufflingGrouping
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

 ///////////////////////////////////////////////////////////////////////////////
 //              Constant Time Stateless Shuffling and Grouping               //
 //         Copyright (c) 2023 Electronic Arts Inc. All rights reserved.      //
 ///////////////////////////////////////////////////////////////////////////////

#pragma once

// MurmurHash code was taken from https://sites.google.com/site/murmurhash/
//-----------------------------------------------------------------------------
// MurmurHash2A, by Austin Appleby
// Note - This code makes a few assumptions about how your machine behaves -
// 1. We can read a 4-byte value from any address without crashing
// 2. sizeof(int) == 4
// And it has a few limitations -
// 1. It will not work incrementally.
// 2. It will not produce the same results on little-endian and big-endian
//    machines.
#define mmix(h,k) { k *= m; k ^= k >> r; k *= m; h *= m; h ^= k; }
uint32_t MurmurHash2A(const void* key, int32_t len, uint32_t seed) {
  // 'm' and 'r' are mixing constants generated offline.
  // They're not really 'magic', they just happen to work well.
  const uint32_t m = 0x5bd1e995;
  const int32_t r = 24;
  uint32_t l = len;

  // Initialize the hash to a 'random' value
  uint32_t h = seed ^ len;

  // Mix 4 bytes at a time into the hash
  const uint8_t* data = (const uint8_t*)key;
  while (len >= 4) {
    uint32_t k = *(uint32_t*)data;
    mmix(h,k);
    data += 4;
    len -= 4;
  }

  // Handle the last few bytes of the input array
  uint32_t t = 0;
	switch(len) {
    case 3: t ^= (uint32_t)data[2] << 16;
    case 2: t ^= (uint32_t)data[1] << 8;
    case 1: t ^= (uint32_t)data[0];
	};
	mmix(h,t);
	mmix(h,l);

  // Do a few final mixes of the hash to ensure the last few
  // bytes are well-incorporated.
  h ^= h >> 13;
  h *= m;
  h ^= h >> 15;
  return h;
}

class StatelessShuffle {
  public:
    void SetSeed(uint32_t seed) {
      m_seed = seed;
    }

    void SetItemCount(uint32_t numItems) {
      uint32_t m_nextPow4 = 4;
      uint32_t m_numBits = 0;
      while (numItems > m_nextPow4)
        m_nextPow4 *= 4;
      uint32_t mask = m_nextPow4 - 1;
      while (mask)
      {
        mask = mask >> 1;
        m_numBits++;
      }
      m_halfIndexBits = m_numBits / 2;
      m_halfIndexBitsMask = (1 << m_halfIndexBits) - 1;
    }

    uint32_t IndexToShuffledIndex(uint32_t index) {
      return Encrypt(index);
    }

    uint32_t ShuffledIndexToIndex(uint32_t index) {
      return Decrypt(index);
    }

  private:
    uint32_t Encrypt(uint32_t index) {
      uint32_t left = (index >> m_halfIndexBits);
      uint32_t right = index & m_halfIndexBitsMask;

      for (uint32_t i = 0; i < 4; ++i) {
        uint32_t newLeft = right;
        uint32_t newRight = left ^ RoundFunction(right);
        left = newLeft;
        right = newRight;
      }

      return (left << m_halfIndexBits) | right;
    }

    uint32_t Decrypt(uint32_t index) {
      uint32_t left = (index >> m_halfIndexBits);
      uint32_t right = index & m_halfIndexBitsMask;

      for (uint32_t i = 0; i < 4; ++i) {
        uint32_t newRight = left;
        uint32_t newLeft = right ^ RoundFunction(left);
        left = newLeft;
        right = newRight;
      }

      return (left << m_halfIndexBits) | right;
    }

    uint32_t RoundFunction(uint32_t x) {
      // This is a function of both the input, as well as the key
      return (pcg_hash(x ^ m_seed)) & m_halfIndexBitsMask;
    }

    // https://www.pcg-random.org/
    // https://www.reedbeta.com/blog/hash-functions-for-gpu-rendering/
    uint32_t pcg_hash(uint32_t input) {
      uint32_t state = input * 747796405u + 2891336453u;
      uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
      return (word >> 22u) ^ word;
    }

    uint32_t m_halfIndexBits = 0;
    uint32_t m_halfIndexBitsMask = 0;
    uint32_t m_seed = 0;  // The "random seed" that determines ordering
};
