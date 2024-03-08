# AHA Project 

### The team:
- Disha R M - PES1UG21EC089
- Ayush Kumar N - PES1UG21EC062
- Amogh S Rao - PES1UG21EC033
- Aman M Halyal - PES1UG21EC032
- Akshat V A - PES1UG21EC028

## SIMON CIPHER

The Simon cipher is a lightweight block cipher designed for efficient hardware and software implementations, especially in resource-constrained environments such as embedded systems, wireless sensor networks, and Internet of Things (IoT) devices.

The Simon cipher was created by researchers at the National Security Agency (NSA) and was first published in 2013. It was designed to achieve high performance while maintaining a high level of security, making it suitable for applications where energy efficiency and low computational complexity are essential.

Here are some key features of the Simon cipher:

1. Block Size and Key Size: Simon supports various block sizes and key sizes, including 64-bit and 128-bit blocks, with key sizes ranging from 64 bits to 256 bits.

2. Structure: The Simon cipher is a Feistel network with a specific number of rounds depending on the block size and key size. For example, Simon64/128 (64-bit block, 128-bit key) has 32 rounds.

3. Round Function: The round function of the Simon cipher is based on bitwise operations, including XOR, bit rotations, and additions modulo 2^n (where n is the word size, typically 16, 24, 32, or 64 bits).

4. Lightweight and Efficient: The Simon cipher is designed to be lightweight and efficient, with a low memory footprint and low computational complexity, making it suitable for resource-constrained devices.

5. Security: The Simon cipher is designed to provide a high level of security against various types of attacks, including differential and linear cryptanalysis. However, it is relatively new, and its security has not been as thoroughly analyzed as established ciphers like AES.

The Simon cipher was designed to complement the NSA's other lightweight cipher, called Speck, which is a substitution-permutation network (SPN) based cipher. Together, Simon and Speck were intended to provide lightweight and efficient cryptographic algorithms for a wide range of applications, particularly in the IoT and embedded systems domains.

While the Simon cipher has gained some attention and adoption in certain applications, it is still considered a relatively new cipher, and its widespread adoption and long-term security analysis are ongoing

## Design Code

```cpp
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
```
## Testbench

```cpp
#include <systemc.h>
#include "simon_cipher.h" // Include the SimonCipher module

SC_MODULE(TestBench) {
    sc_signal<bool> clk, reset;
    sc_signal<sc_uint<WORD_SIZE> > key_signal[KEY_SIZE];
    sc_signal<sc_uint<WORD_SIZE * 2> > plaintext_signal, ciphertext_signal, decrypted_signal;

    SimonCipher* simon_cipher_encrypt;
    SimonCipher* simon_cipher_decrypt;

    void clock_generator() {
        clk = 0;
        while (true) {
#if defined(NC_SYSTEMC)
            clk.write(!clk.read());
#else
            clk = !clk;
#endif
            sc_start(1, SC_NS);
        }
    }

    void test_simon_cipher() {
        // Initialize key and plaintext values
        std::vector<uint16_t> key = {0x1918, 0x1110, 0x0908, 0x0100, 0x0706, 0x0504, 0x0302, 0x0100};
        std::vector<uint16_t> plaintext = {0x6565, 0x6877};

        for (int i = 0; i < KEY_SIZE; i++) {
            key_signal[i].write(key[i]);
        }
        plaintext_signal.write((sc_uint<WORD_SIZE * 2>)((plaintext[1] << WORD_SIZE) | plaintext[0]));

        sc_start(2, SC_NS);
        reset.write(1);
        sc_start(2, SC_NS);
        reset.write(0);

        sc_start(ROUNDS + 1, SC_NS);

        std::cout << "Plaintext: " << std::hex << plaintext_signal.read() << std::endl;
        std::cout << "Ciphertext: " << std::hex << ciphertext_signal.read() << std::endl;
        std::cout << "Decrypted plaintext: " << std::hex << decrypted_signal.read() << std::endl;

        sc_stop();
    }

    SC_CTOR(TestBench) {
        simon_cipher_encrypt = new SimonCipher("SimonCipherEncrypt");
        simon_cipher_decrypt = new SimonCipher("SimonCipherDecrypt");

        simon_cipher_encrypt->clk(clk);
        simon_cipher_encrypt->reset(reset);
        simon_cipher_decrypt->clk(clk);
        simon_cipher_decrypt->reset(reset);

        for (int i = 0; i < KEY_SIZE; i++) {
            simon_cipher_encrypt->key[i](key_signal[i]);
            simon_cipher_decrypt->key[i](key_signal[i]);
        }

        simon_cipher_encrypt->plaintext(plaintext_signal);
        simon_cipher_encrypt->ciphertext(ciphertext_signal);
        simon_cipher_decrypt->plaintext(ciphertext_signal);
        simon_cipher_decrypt->ciphertext(decrypted_signal);

        SC_THREAD(clock_generator);
        SC_THREAD(test_simon_cipher);
    }

    ~TestBench() {
        delete simon_cipher_encrypt;
        delete simon_cipher_decrypt;
    }
};

int sc_main(int argc, char* argv[]) {
    TestBench tb("TestBench");
    sc_start();
    return 0;
}
```

