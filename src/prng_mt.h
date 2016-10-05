#ifndef PRNG_MT_H
#define PRNG_MT_H

// mersenne twister yay

class PRNG_MT {
 private:
  unsigned long mt[624];
  unsigned int rng_v;

  void generate_next_set() {
    static const unsigned long table[2] = {0, 2567483615UL};
    unsigned long v;

    for (int i = 0; i < 623; ++i) {
      v = (mt[i] & 0x80000000) | (mt[i + 1] & 0x7fffffff);

      mt[i] = mt[(i + 397) % 624] ^ (v >> 1) ^ table[v & 1];
    }

    v = (mt[623] & 0x80000000) | (mt[0] & 0x7fffffff);
    mt[623] = mt[396] ^ (v >> 1);
    if (v & 1) {
      mt[623] ^= table[1];
    }

    rng_v = 0;
  }

 public:
  void seed(unsigned long seed_v) {
    mt[0] = seed_v;
    for (unsigned long i = 1; i < 624; ++i) {
      mt[i] = 1812433253UL * (mt[i - 1] ^ (mt[i - 1] >> 30)) + i;
    }

    generate_next_set();
  }

  unsigned long rand_long() {
    if (rng_v >= 624) {
      generate_next_set();
    }

    unsigned long v = mt[rng_v++];

    v ^= (v >> 11);
    v ^= (v << 7) & 0x9d2c5680;
    v ^= (v << 15) & 0xefc60000;
    v ^= (v >> 18);

    return v;
  }

  PRNG_MT() { rng_v = 0; }

  PRNG_MT(unsigned long seed_v) { seed(seed_v); }

  ~PRNG_MT() {}
};

#endif
