#include <systemc.h>

// Simon block cipher constants
const int ROUNDS = 32; // Number of rounds (32 for Simon64/128)
const int WORD_SIZE = 16; // Word size in bits (16 for Simon64/128)
const int KEY_SIZE = 8; // Key size in words (8 for Simon64/128)

// Rotate left operation
uint16_t rotl(uint16_t x, uint16_t n) {
    return (x << n) | (x >> (WORD_SIZE - n));
}

SC_MODULE(SimonCipher) {
    sc_in<bool> clk;
    sc_in<bool> reset;
    sc_in<sc_uint<WORD_SIZE> > key[KEY_SIZE];
    sc_in<sc_uint<WORD_SIZE * 2> > plaintext;
    sc_out<sc_uint<WORD_SIZE * 2> > ciphertext;

    void simon_round(sc_uint<WORD_SIZE * 2>& x, const sc_uint<WORD_SIZE>& k, bool encrypt) {
        sc_uint<WORD_SIZE * 2> tmp = x;
        x[WORD_SIZE * 2 - 1, WORD_SIZE] ^= k ^ rotl(tmp[WORD_SIZE - 1, 0], -3) ^ rotl(tmp[WORD_SIZE - 1, 0], -8);
        x[WORD_SIZE - 1, 0] ^= x[WORD_SIZE * 2 - 1, WORD_SIZE] ^ rotl(x[WORD_SIZE * 2 - 1, WORD_SIZE], -3) ^ rotl(tmp[WORD_SIZE * 2 - 1, WORD_SIZE], -8);
        if (!encrypt) {
            x = tmp;
        }
    }

    void simon_cipher() {
        sc_uint<WORD_SIZE * 2> x = plaintext.read();
        for (int i = 0; i < ROUNDS; i++) {
            simon_round(x, key[i % KEY_SIZE].read(), true);
        }
        ciphertext.write(x);
    }

    SC_CTOR(SimonCipher) {
        SC_METHOD(simon_cipher);
        sensitive << clk.pos();
        async_reset_signal_is(reset, false);
    }
};
