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
