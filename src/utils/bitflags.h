static inline bool bf_has(uint32_t f, uint8_t pos) { return (f & (1 << pos)) != 0u; }
static inline void bf_set(uint32_t *f, uint8_t pos) { *f |= (1 << pos); }
static inline void bf_clr(uint32_t *f, uint8_t pos) { *f &= ~(1 << pos); }
static inline void bf_tgl(uint32_t *f, uint8_t pos) { *f ^= (1 << pos); }
static inline void bf_put(uint32_t *f, uint8_t pos, bool on)
{
    *f = on ? (*f | (1 << pos)) : (*f & ~(1 << pos));
}
