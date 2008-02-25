// $Id$

#include <assert.h>
#include <stdint.h>
#include <sys/time.h>

#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <numeric>
#include <cmath>

#include <gcrypt.h>

#include <mcrypt.h>

// *** Speedtest Parameters ***

// speed test different buffer sizes in this range
const unsigned int buffermin = 16;
const unsigned int buffermax = 16 * 65536;
const unsigned int repeatsize = 65536;
const unsigned int measureruns = 1;

/// Time is measured using gettimeofday()
inline double timestamp()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 0.000001;
}

// *** Global Buffers and Settings for the Speedtest Functions ***

char	enckey[32];	/// 256 bit encryption key
char	enciv[16];	/// 16 byte initialization vector if needed.

char	buffer[buffermax];	/// encryption buffer
unsigned int bufferlen;		/// currently tested buffer length

// *** Test Functions for libgcrypt ***

void test_libgcrypt_rijndael_ecb()
{
    gcry_cipher_hd_t encctx;
    gcry_cipher_open(&encctx, GCRY_CIPHER_RIJNDAEL256, GCRY_CIPHER_MODE_ECB, 0);
    gcry_cipher_setkey(encctx, (uint8_t*)enckey, 32);
    gcry_cipher_encrypt(encctx, buffer, bufferlen, buffer, bufferlen);
    gcry_cipher_close(encctx);

    gcry_cipher_hd_t decctx;
    gcry_cipher_open(&decctx, GCRY_CIPHER_RIJNDAEL256, GCRY_CIPHER_MODE_ECB, 0);
    gcry_cipher_setkey(decctx, (uint8_t*)enckey, 32);
    gcry_cipher_decrypt(decctx, buffer, bufferlen, buffer, bufferlen);
    gcry_cipher_close(decctx);
}

void test_libgcrypt_serpent_ecb()
{
    gcry_cipher_hd_t encctx;
    gcry_cipher_open(&encctx, GCRY_CIPHER_SERPENT256, GCRY_CIPHER_MODE_ECB, 0);
    gcry_cipher_setkey(encctx, (uint8_t*)enckey, 32);
    gcry_cipher_encrypt(encctx, buffer, bufferlen, buffer, bufferlen);
    gcry_cipher_close(encctx);

    gcry_cipher_hd_t decctx;
    gcry_cipher_open(&decctx, GCRY_CIPHER_SERPENT256, GCRY_CIPHER_MODE_ECB, 0);
    gcry_cipher_setkey(decctx, (uint8_t*)enckey, 32);
    gcry_cipher_decrypt(decctx, buffer, bufferlen, buffer, bufferlen);
    gcry_cipher_close(decctx);
}

void test_libgcrypt_camellia_ecb()
{
    gcry_cipher_hd_t encctx;
    gcry_cipher_open(&encctx, GCRY_CIPHER_CAMELLIA256, GCRY_CIPHER_MODE_ECB, 0);
    gcry_cipher_setkey(encctx, (uint8_t*)enckey, 32);
    gcry_cipher_encrypt(encctx, buffer, bufferlen, buffer, bufferlen);
    gcry_cipher_close(encctx);

    gcry_cipher_hd_t decctx;
    gcry_cipher_open(&decctx, GCRY_CIPHER_CAMELLIA256, GCRY_CIPHER_MODE_ECB, 0);
    gcry_cipher_setkey(decctx, (uint8_t*)enckey, 32);
    gcry_cipher_decrypt(decctx, buffer, bufferlen, buffer, bufferlen);
    gcry_cipher_close(decctx);
}

void test_libgcrypt_twofish_ecb()
{
    gcry_cipher_hd_t encctx;
    gcry_cipher_open(&encctx, GCRY_CIPHER_TWOFISH, GCRY_CIPHER_MODE_ECB, 0);
    gcry_cipher_setkey(encctx, (uint8_t*)enckey, 32);
    gcry_cipher_encrypt(encctx, buffer, bufferlen, buffer, bufferlen);
    gcry_cipher_close(encctx);

    gcry_cipher_hd_t decctx;
    gcry_cipher_open(&decctx, GCRY_CIPHER_TWOFISH, GCRY_CIPHER_MODE_ECB, 0);
    gcry_cipher_setkey(decctx, (uint8_t*)enckey, 32);
    gcry_cipher_decrypt(decctx, buffer, bufferlen, buffer, bufferlen);
    gcry_cipher_close(decctx);
}

void test_libgcrypt_blowfish_ecb()
{
    gcry_cipher_hd_t encctx;
    gcry_cipher_open(&encctx, GCRY_CIPHER_BLOWFISH, GCRY_CIPHER_MODE_ECB, 0);
    gcry_cipher_setkey(encctx, (uint8_t*)enckey, 16);
    gcry_cipher_encrypt(encctx, buffer, bufferlen, buffer, bufferlen);
    gcry_cipher_close(encctx);

    gcry_cipher_hd_t decctx;
    gcry_cipher_open(&decctx, GCRY_CIPHER_BLOWFISH, GCRY_CIPHER_MODE_ECB, 0);
    gcry_cipher_setkey(decctx, (uint8_t*)enckey, 16);
    gcry_cipher_decrypt(decctx, buffer, bufferlen, buffer, bufferlen);
    gcry_cipher_close(decctx);
}

void test_libgcrypt_cast5_ecb()
{
    gcry_cipher_hd_t encctx;
    gcry_cipher_open(&encctx, GCRY_CIPHER_CAST5, GCRY_CIPHER_MODE_ECB, 0);
    gcry_cipher_setkey(encctx, (uint8_t*)enckey, 16);
    gcry_cipher_encrypt(encctx, buffer, bufferlen, buffer, bufferlen);
    gcry_cipher_close(encctx);

    gcry_cipher_hd_t decctx;
    gcry_cipher_open(&decctx, GCRY_CIPHER_CAST5, GCRY_CIPHER_MODE_ECB, 0);
    gcry_cipher_setkey(decctx, (uint8_t*)enckey, 16);
    gcry_cipher_decrypt(decctx, buffer, bufferlen, buffer, bufferlen);
    gcry_cipher_close(decctx);
}

void test_libgcrypt_3des_ecb()
{
    gcry_cipher_hd_t encctx;
    gcry_cipher_open(&encctx, GCRY_CIPHER_3DES, GCRY_CIPHER_MODE_ECB, 0);
    gcry_cipher_setkey(encctx, (uint8_t*)enckey, 24);
    gcry_cipher_encrypt(encctx, buffer, bufferlen, buffer, bufferlen);
    gcry_cipher_close(encctx);

    gcry_cipher_hd_t decctx;
    gcry_cipher_open(&decctx, GCRY_CIPHER_3DES, GCRY_CIPHER_MODE_ECB, 0);
    gcry_cipher_setkey(decctx, (uint8_t*)enckey, 24);
    gcry_cipher_decrypt(decctx, buffer, bufferlen, buffer, bufferlen);
    gcry_cipher_close(decctx);
}

// *** Test Functions for libmcrypt ***

void test_libmcrypt_rijndael_ecb()
{
    // note: MCRYPT_RIJNDAEL_128 means blocksize 128 _not_ keysize 128 bits

    MCRYPT encctx = mcrypt_module_open(MCRYPT_RIJNDAEL_128, NULL, MCRYPT_ECB, NULL);
    mcrypt_generic_init(encctx, enckey, 32, NULL);
    mcrypt_generic(encctx, buffer, bufferlen);
    mcrypt_generic_end(encctx);

    MCRYPT decctx = mcrypt_module_open(MCRYPT_RIJNDAEL_128, NULL, MCRYPT_ECB, NULL);
    mcrypt_generic_init(decctx, enckey, 32, NULL);
    mdecrypt_generic(decctx, buffer, bufferlen);
    mcrypt_generic_end(decctx);
}

void test_libmcrypt_serpent_ecb()
{
    MCRYPT encctx = mcrypt_module_open(MCRYPT_SERPENT, NULL, MCRYPT_ECB, NULL);
    mcrypt_generic_init(encctx, enckey, 32, NULL);
    mcrypt_generic(encctx, buffer, bufferlen);
    mcrypt_generic_end(encctx);

    MCRYPT decctx = mcrypt_module_open(MCRYPT_SERPENT, NULL, MCRYPT_ECB, NULL);
    mcrypt_generic_init(decctx, enckey, 32, NULL);
    mdecrypt_generic(decctx, buffer, bufferlen);
    mcrypt_generic_end(decctx);
}

void test_libmcrypt_twofish_ecb()
{
    MCRYPT encctx = mcrypt_module_open(MCRYPT_TWOFISH, NULL, MCRYPT_ECB, NULL);
    mcrypt_generic_init(encctx, enckey, 32, NULL);
    mcrypt_generic(encctx, buffer, bufferlen);
    mcrypt_generic_end(encctx);

    MCRYPT decctx = mcrypt_module_open(MCRYPT_TWOFISH, NULL, MCRYPT_ECB, NULL);
    mcrypt_generic_init(decctx, enckey, 32, NULL);
    mdecrypt_generic(decctx, buffer, bufferlen);
    mcrypt_generic_end(decctx);
}

void test_libmcrypt_cast6_ecb()
{
    MCRYPT encctx = mcrypt_module_open(MCRYPT_CAST_256, NULL, MCRYPT_ECB, NULL);
    mcrypt_generic_init(encctx, enckey, 32, NULL);
    mcrypt_generic(encctx, buffer, bufferlen);
    mcrypt_generic_end(encctx);

    MCRYPT decctx = mcrypt_module_open(MCRYPT_CAST_256, NULL, MCRYPT_ECB, NULL);
    mcrypt_generic_init(decctx, enckey, 32, NULL);
    mdecrypt_generic(decctx, buffer, bufferlen);
    mcrypt_generic_end(decctx);
}

void test_libmcrypt_xtea_ecb()
{
    MCRYPT encctx = mcrypt_module_open(MCRYPT_XTEA, NULL, MCRYPT_ECB, NULL);
    mcrypt_generic_init(encctx, enckey, 16, NULL);
    mcrypt_generic(encctx, buffer, bufferlen);
    mcrypt_generic_end(encctx);

    MCRYPT decctx = mcrypt_module_open(MCRYPT_XTEA, NULL, MCRYPT_ECB, NULL);
    mcrypt_generic_init(decctx, enckey, 16, NULL);
    mdecrypt_generic(decctx, buffer, bufferlen);
    mcrypt_generic_end(decctx);
}

void test_libmcrypt_saferplus_ecb()
{
    MCRYPT encctx = mcrypt_module_open(MCRYPT_SAFERPLUS, NULL, MCRYPT_ECB, NULL);
    mcrypt_generic_init(encctx, enckey, 32, NULL);
    mcrypt_generic(encctx, buffer, bufferlen);
    mcrypt_generic_end(encctx);

    MCRYPT decctx = mcrypt_module_open(MCRYPT_SAFERPLUS, NULL, MCRYPT_ECB, NULL);
    mcrypt_generic_init(decctx, enckey, 32, NULL);
    mdecrypt_generic(decctx, buffer, bufferlen);
    mcrypt_generic_end(decctx);
}

void test_libmcrypt_loki97_ecb()
{
    MCRYPT encctx = mcrypt_module_open(MCRYPT_LOKI97, NULL, MCRYPT_ECB, NULL);
    mcrypt_generic_init(encctx, enckey, 32, NULL);
    mcrypt_generic(encctx, buffer, bufferlen);
    mcrypt_generic_end(encctx);

    MCRYPT decctx = mcrypt_module_open(MCRYPT_LOKI97, NULL, MCRYPT_ECB, NULL);
    mcrypt_generic_init(decctx, enckey, 32, NULL);
    mdecrypt_generic(decctx, buffer, bufferlen);
    mcrypt_generic_end(decctx);
}

void test_libmcrypt_blowfish_ecb()
{
    MCRYPT encctx = mcrypt_module_open(MCRYPT_BLOWFISH, NULL, MCRYPT_ECB, NULL);
    mcrypt_generic_init(encctx, enckey, 32, NULL);
    mcrypt_generic(encctx, buffer, bufferlen);
    mcrypt_generic_end(encctx);

    MCRYPT decctx = mcrypt_module_open(MCRYPT_BLOWFISH, NULL, MCRYPT_ECB, NULL);
    mcrypt_generic_init(decctx, enckey, 32, NULL);
    mdecrypt_generic(decctx, buffer, bufferlen);
    mcrypt_generic_end(decctx);
}

void test_libmcrypt_gost_ecb()
{
    MCRYPT encctx = mcrypt_module_open(MCRYPT_GOST, NULL, MCRYPT_ECB, NULL);
    mcrypt_generic_init(encctx, enckey, 32, NULL);
    mcrypt_generic(encctx, buffer, bufferlen);
    mcrypt_generic_end(encctx);

    MCRYPT decctx = mcrypt_module_open(MCRYPT_GOST, NULL, MCRYPT_ECB, NULL);
    mcrypt_generic_init(decctx, enckey, 32, NULL);
    mdecrypt_generic(decctx, buffer, bufferlen);
    mcrypt_generic_end(decctx);
}

void test_libmcrypt_cast5_ecb()
{
    MCRYPT encctx = mcrypt_module_open(MCRYPT_CAST_128, NULL, MCRYPT_ECB, NULL);
    mcrypt_generic_init(encctx, enckey, 16, NULL);
    mcrypt_generic(encctx, buffer, bufferlen);
    mcrypt_generic_end(encctx);

    MCRYPT decctx = mcrypt_module_open(MCRYPT_CAST_128, NULL, MCRYPT_ECB, NULL);
    mcrypt_generic_init(decctx, enckey, 16, NULL);
    mdecrypt_generic(decctx, buffer, bufferlen);
    mcrypt_generic_end(decctx);
}

void test_libmcrypt_3des_ecb()
{
    MCRYPT encctx = mcrypt_module_open(MCRYPT_3DES, NULL, MCRYPT_ECB, NULL);
    mcrypt_generic_init(encctx, enckey, 16, NULL);
    mcrypt_generic(encctx, buffer, bufferlen);
    mcrypt_generic_end(encctx);

    MCRYPT decctx = mcrypt_module_open(MCRYPT_3DES, NULL, MCRYPT_ECB, NULL);
    mcrypt_generic_init(decctx, enckey, 16, NULL);
    mdecrypt_generic(decctx, buffer, bufferlen);
    mcrypt_generic_end(decctx);
}

// *** main() and run_test() ***

/**
 * This function will run a test routine multiple times with different buffer
 * sizes configured. It measures the time required to encrypt a number of
 * bytes. The average time and standard deviation are calculated and written to
 * a log file for gnuplot.
 */

template <void (*testfunc)()>
void run_test(const char* logfile)
{
    std::cout << "Speed testing for " << logfile << "\n";

    // Save the time required for each run.
    std::map<unsigned int, std::vector<double> > timelog;

    for(unsigned int fullrun = 0; fullrun < measureruns; ++fullrun)
    {
	for(unsigned int bufflen = buffermin; bufflen <= buffermax; bufflen *= 2)
	{
	    // because small time measurements are inaccurate, repeat very fast
	    // tests until the same amount of data is encrypted as in the large
	    // tests.
	    unsigned int repeat = repeatsize / bufflen;
	    if (repeat < 1) repeat = 1;

	    std::cout << "Test: bufflen " << bufflen << " repeat " << repeat << "\n";

	    bufferlen = bufflen;

	    // fill buffer
	    for(unsigned int i = 0; i < bufferlen; ++i)
		buffer[i] = (char)i;

	    double ts1 = timestamp();

	    for(unsigned int testrun = 0; testrun < repeat; ++testrun)
	    {
		testfunc();
	    }

	    double ts2 = timestamp();

	    // check buffer status after repeated en/decryption
	    for(unsigned int i = 0; i < bufferlen; ++i)
		assert(buffer[i] == (char)i);

	    timelog[bufferlen].push_back( (ts2 - ts1) / (double)repeat );
	}
    }

    // Calculate and output statistics.
    std::ofstream of (logfile);

    for(std::map<unsigned int, std::vector<double> >::const_iterator ti = timelog.begin();
	ti != timelog.end(); ++ti)
    {
	const std::vector<double>& timelist = ti->second;

	double average = std::accumulate(timelist.begin(), timelist.end(), 0.0) / timelist.size();

	double variance = 0.0;
	for(unsigned int i = 0; i < timelist.size(); ++i)
	    variance += (timelist[i] - average) * (timelist[i] - average);
	variance = variance / (timelist.size() - 1);

	double stddev = std::sqrt(variance);

	double vmin = *std::min_element(timelist.begin(), timelist.end());
	double vmax = *std::max_element(timelist.begin(), timelist.end());

	of << std::setprecision(16);
	of << ti->first << " " << average << " " << stddev << " " << vmin << " " << vmax << "\n";
    }
}

int main()
{
    // Initialize all cryptographic libaries

    const char *gcryversion = gcry_check_version(GCRYPT_VERSION);
    assert(gcryversion);

    // Create (somewhat) random encryption key and initialization vector

    srand(time(NULL));

    for(unsigned int i = 0; i < sizeof(enckey); ++i)
	enckey[i] = rand();

    for(unsigned int i = 0; i < sizeof(enciv); ++i)
	enciv[i] = rand();

    // Run speed tests

#if 0
    run_test<test_libgcrypt_rijndael_ecb>("gcrypt-rijndael-ecb.txt");
    run_test<test_libgcrypt_serpent_ecb>("gcrypt-serpent-ecb.txt");
    run_test<test_libgcrypt_twofish_ecb>("gcrypt-twofish-ecb.txt");
    run_test<test_libgcrypt_camellia_ecb>("gcrypt-camellia-ecb.txt");
    run_test<test_libgcrypt_blowfish_ecb>("gcrypt-blowfish-ecb.txt");
    run_test<test_libgcrypt_cast5_ecb>("gcrypt-cast5-ecb.txt");
    run_test<test_libgcrypt_3des_ecb>("gcrypt-3des-ecb.txt");
#endif

    run_test<test_libmcrypt_rijndael_ecb>("mcrypt-rijndael-ecb.txt");
    run_test<test_libmcrypt_serpent_ecb>("mcrypt-serpent-ecb.txt");
    run_test<test_libmcrypt_twofish_ecb>("mcrypt-twofish-ecb.txt");
    run_test<test_libmcrypt_cast6_ecb>("mcrypt-cast6-ecb.txt");
    run_test<test_libmcrypt_xtea_ecb>("mcrypt-xtea-ecb.txt");
    run_test<test_libmcrypt_saferplus_ecb>("mcrypt-saferplus-ecb.txt");
    run_test<test_libmcrypt_loki97_ecb>("mcrypt-loki97-ecb.txt");
    run_test<test_libmcrypt_blowfish_ecb>("mcrypt-blowfish-ecb.txt");
    run_test<test_libmcrypt_gost_ecb>("mcrypt-gost-ecb.txt");
    run_test<test_libmcrypt_cast5_ecb>("mcrypt-cast5-ecb.txt");
    run_test<test_libmcrypt_3des_ecb>("mcrypt-3des-ecb.txt");
}
