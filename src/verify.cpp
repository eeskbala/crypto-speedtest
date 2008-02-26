// $Id$

#include <iostream>
#include <fstream>
#include <sys/time.h>
#include <assert.h>
#include <map>
#include <vector>
#include <numeric>
#include <math.h>

#if HAVE_LIBTOMCRYPT
#include <tomcrypt.h>
// remove some macros slashing with other libraries
#undef byte
#undef XTEA
#undef DES
#endif

#if HAVE_LIBGCRYPT
#include <gcrypt.h>
#endif

#if HAVE_LIBMCRYPT
#include <mcrypt.h>
#endif

#if HAVE_BOTAN
#include <botan/botan.h>
#include <botan/aes.h>
#include <botan/serpent.h>
#include <botan/twofish.h>
#include <botan/des.h>
#endif

#if HAVE_CRYPTOPP
#include <crypto++/modes.h>
#include <crypto++/rijndael.h>
#include <crypto++/serpent.h>
#include <crypto++/twofish.h>
#include <crypto++/des.h>
#endif

#if HAVE_OPENSSL
#define NCOMPAT
#include <openssl/aes.h>
#include <openssl/des.h>
#endif

#if HAVE_LIBNETTLE
extern "C" {
#include <nettle/aes.h>
#include <nettle/serpent.h>
#include <nettle/des.h>
}
#endif

#if HAVE_LIBBEECRYPT
#include <beecrypt/aes.h>
#endif

#include "rijndael.h"
#include "serpent-gladman.h"
#include "serpent.h"

// *** Verfication Parameters ***

const unsigned int bufferlen = 8192*16;

// *** Global Buffers and Settings for the Verify Functions ***

char enckey[32];

// *** Tool Functions to Fill, Check and Compare Buffers ***

void fill_buffer(void *buffer, unsigned int bufferlen)
{
    uint8_t* cbuffer = (uint8_t*)buffer;
    for(unsigned int i = 0; i < bufferlen; ++i)
	cbuffer[i] = (uint8_t)i;
}

void compare_buffers(void *buffer1, void* buffer2, unsigned int bufferlen)
{
    uint8_t* cbuffer1 = (uint8_t*)buffer1;
    uint8_t* cbuffer2 = (uint8_t*)buffer2;
    for(unsigned int i = 0; i < bufferlen; ++i)
	assert(cbuffer1[i] == cbuffer2[i]);
}

void check_buffer(void *buffer, unsigned int bufferlen)
{
    uint8_t* cbuffer = (uint8_t*)buffer;
    for(unsigned int i = 0; i < bufferlen; ++i)
	assert(cbuffer[i] == (uint8_t)i);
}

// *** Verify Rijndael Implementations

void verify_rijndael_ecb()
{
    // Custom Implementation

    char buffer_custom[bufferlen];
    fill_buffer(buffer_custom, bufferlen);

    {
	RijndaelEncryptECB encctx;
	encctx.set_key((uint8_t*)enckey, 32);
	encctx.encrypt(buffer_custom, buffer_custom, bufferlen);
    }

#if HAVE_LIBGCRYPT
    { // libgcrypt

	char buffer_gcrypt[bufferlen];
	fill_buffer(buffer_gcrypt, bufferlen);

	{
	    gcry_cipher_hd_t encctx;
	    gcry_cipher_open(&encctx, GCRY_CIPHER_RIJNDAEL256, GCRY_CIPHER_MODE_ECB, 0);
	    gcry_cipher_setkey(encctx, (uint8_t*)enckey, 32);
	    gcry_cipher_encrypt(encctx, buffer_gcrypt, bufferlen, buffer_gcrypt, bufferlen);
	    gcry_cipher_close(encctx);
	}

	compare_buffers(buffer_custom, buffer_gcrypt, bufferlen);

	{
	    gcry_cipher_hd_t decctx;
	    gcry_cipher_open(&decctx, GCRY_CIPHER_RIJNDAEL256, GCRY_CIPHER_MODE_ECB, 0);
	    gcry_cipher_setkey(decctx, (uint8_t*)enckey, 32);
	    gcry_cipher_decrypt(decctx, buffer_gcrypt, bufferlen, buffer_gcrypt, bufferlen);
	    gcry_cipher_close(decctx);
	}

	check_buffer(buffer_gcrypt, bufferlen);
    }
#endif

#if HAVE_LIBMCRYPT
    { // libmcrypt

	char buffer_mcrypt[bufferlen];
	fill_buffer(buffer_mcrypt, bufferlen);

	{
	    MCRYPT encctx = mcrypt_module_open(MCRYPT_RIJNDAEL_128, NULL, MCRYPT_ECB, NULL);
	    mcrypt_generic_init(encctx, enckey, 32, NULL);
	    mcrypt_generic(encctx, buffer_mcrypt, bufferlen);
	    mcrypt_generic_end(encctx);
	}

	compare_buffers(buffer_custom, buffer_mcrypt, bufferlen);

	{
	    MCRYPT decctx = mcrypt_module_open(MCRYPT_RIJNDAEL_128, NULL, MCRYPT_ECB, NULL);
	    mcrypt_generic_init(decctx, enckey, 32, NULL);
	    mdecrypt_generic(decctx, buffer_mcrypt, bufferlen);
	    mcrypt_generic_end(decctx);
	}

	check_buffer(buffer_mcrypt, bufferlen);
    }
#endif

#if HAVE_BOTAN
    { // Botan

	char buffer_botan[bufferlen];
	fill_buffer(buffer_botan, bufferlen);

	{
	    Botan::AES_256 encctx;
	    encctx.set_key((Botan::byte*)enckey, 32);

	    for(unsigned int p = 0; p < bufferlen; p += encctx.BLOCK_SIZE)
		encctx.encrypt((Botan::byte*)buffer_botan + p);
	}

	compare_buffers(buffer_custom, buffer_botan, bufferlen);

	{
	    Botan::AES_256 decctx;
	    decctx.set_key((Botan::byte*)enckey, 32);

	    for(unsigned int p = 0; p < bufferlen; p += decctx.BLOCK_SIZE)
		decctx.decrypt((Botan::byte*)buffer_botan + p);
	}

	check_buffer(buffer_botan, bufferlen);
    }
#endif

#if HAVE_CRYPTOPP
    { // Crypto++

	char buffer_cryptopp[bufferlen];
	fill_buffer(buffer_cryptopp, bufferlen);

	{
	    CryptoPP::ECB_Mode<CryptoPP::Rijndael>::Encryption encctx;
	    encctx.SetKey((uint8_t*)enckey, 32);

	    encctx.ProcessData((uint8_t*)buffer_cryptopp, (uint8_t*)buffer_cryptopp, bufferlen);
	}

	compare_buffers(buffer_custom, buffer_cryptopp, bufferlen);

	{
	    CryptoPP::ECB_Mode<CryptoPP::Rijndael>::Decryption decctx;
	    decctx.SetKey((uint8_t*)enckey, 32);

	    decctx.ProcessData((uint8_t*)buffer_cryptopp, (uint8_t*)buffer_cryptopp, bufferlen);
	}

	check_buffer(buffer_cryptopp, bufferlen);
    }
#endif

#if HAVE_OPENSSL
    { // OpenSSL

	char buffer_openssl[bufferlen];
	fill_buffer(buffer_openssl, bufferlen);

	{
	    AES_KEY aeskey;
	    AES_set_encrypt_key((uint8_t*)enckey, 256, &aeskey);

	    for(unsigned int p = 0; p < bufferlen; p += AES_BLOCK_SIZE)
		AES_encrypt((uint8_t*)buffer_openssl + p, (uint8_t*)buffer_openssl + p, &aeskey);
	}

	compare_buffers(buffer_custom, buffer_openssl, bufferlen);

	{
	    AES_KEY aeskey;
	    AES_set_decrypt_key((uint8_t*)enckey, 256, &aeskey);

	    for(unsigned int p = 0; p < bufferlen; p += AES_BLOCK_SIZE)
		AES_decrypt((uint8_t*)buffer_openssl + p, (uint8_t*)buffer_openssl + p, &aeskey);
	}

	check_buffer(buffer_openssl, bufferlen);
    }
#endif

#if HAVE_LIBNETTLE
    { // Nettle

	char buffer_nettle[bufferlen];
	fill_buffer(buffer_nettle, bufferlen);

	{
	    aes_ctx encctx;
	    aes_set_encrypt_key(&encctx, 32, (uint8_t*)enckey);
	    aes_encrypt(&encctx, bufferlen, (uint8_t*)buffer_nettle, (uint8_t*)buffer_nettle);
	}

	compare_buffers(buffer_custom, buffer_nettle, bufferlen);

	{
	    aes_ctx decctx;
	    aes_set_decrypt_key(&decctx, 32, (uint8_t*)enckey);
	    aes_decrypt(&decctx, bufferlen, (uint8_t*)buffer_nettle, (uint8_t*)buffer_nettle);
	}

	check_buffer(buffer_nettle, bufferlen);
    }
#endif

#if HAVE_LIBBEECRYPT
    { // Beecrypt

	char buffer_beecrypt[bufferlen];
	fill_buffer(buffer_beecrypt, bufferlen);

	{
	    aesParam encctx;
	    aesSetup(&encctx, (uint8_t*)enckey, 256, ENCRYPT);

	    for(unsigned int p = 0; p < bufferlen; p += 16)
		aesEncrypt(&encctx, (uint32_t*)(buffer_beecrypt + p), (uint32_t*)(buffer_beecrypt + p));
	}

	compare_buffers(buffer_custom, buffer_beecrypt, bufferlen);

	{
	    aesParam decctx;
	    aesSetup(&decctx, (uint8_t*)enckey, 256, DECRYPT);

	    for(unsigned int p = 0; p < bufferlen; p += 16)
		aesDecrypt(&decctx, (uint32_t*)(buffer_beecrypt + p), (uint32_t*)(buffer_beecrypt + p));
	}

	check_buffer(buffer_beecrypt, bufferlen);
    }
#endif

#if HAVE_LIBTOMCRYPT
    { // Tomcrypt

	char buffer_tomcrypt[bufferlen];
	fill_buffer(buffer_tomcrypt, bufferlen);

	{
	    symmetric_ECB encctx;
	    ecb_start(find_cipher("rijndael"), (uint8_t*)enckey, 32, 0, &encctx);
	    ecb_encrypt((uint8_t*)buffer_tomcrypt, (uint8_t*)buffer_tomcrypt, bufferlen, &encctx);
	    ecb_done(&encctx);
	}

	compare_buffers(buffer_custom, buffer_tomcrypt, bufferlen);

	{
	    symmetric_ECB decctx;
	    ecb_start(find_cipher("rijndael"), (uint8_t*)enckey, 32, 0, &decctx);
	    ecb_decrypt((uint8_t*)buffer_tomcrypt, (uint8_t*)buffer_tomcrypt, bufferlen, &decctx);
	    ecb_done(&decctx);
	}

	check_buffer(buffer_tomcrypt, bufferlen);
    }
#endif

    // Custom Implementation

    {
	RijndaelDecryptECB decctx;
	decctx.set_key((uint8_t*)enckey, 32);
	decctx.decrypt(buffer_custom, buffer_custom, bufferlen);
    }

    check_buffer(buffer_custom, bufferlen);
}

// *** Verify Serpent Implementations

void verify_serpent_ecb()
{
    // gladman implementation

    char buffer_gladman[bufferlen];
    fill_buffer(buffer_gladman, bufferlen);

    {
	SerpentGladman::EncryptECB encctx;

	encctx.set_key((uint8_t*)enckey, 256);
	encctx.encrypt(buffer_gladman, buffer_gladman, bufferlen);
    }

    { // botan-extracted implementation

	char buffer_mybotan[bufferlen];
	fill_buffer(buffer_mybotan, bufferlen);

	{
	    SerpentBotan::EncryptECB encctx;

	    encctx.set_key((uint8_t*)enckey, 32);
	    encctx.encrypt(buffer_mybotan, buffer_mybotan, bufferlen);
	}

	compare_buffers(buffer_gladman, buffer_mybotan, bufferlen);

	{
	    SerpentBotan::DecryptECB decctx;

	    decctx.set_key((uint8_t*)enckey, 32);
	    decctx.decrypt(buffer_mybotan, buffer_mybotan, bufferlen);
	}

	check_buffer(buffer_mybotan, bufferlen);
    }

#if HAVE_LIBGCRYPT
    { // libgcrypt

	char buffer_gcrypt[bufferlen];
	fill_buffer(buffer_gcrypt, bufferlen);

	{
	    gcry_cipher_hd_t encctx;
	    gcry_cipher_open(&encctx, GCRY_CIPHER_SERPENT256, GCRY_CIPHER_MODE_ECB, 0);
	    gcry_cipher_setkey(encctx, (uint8_t*)enckey, 32);
	    gcry_cipher_encrypt(encctx, buffer_gcrypt, bufferlen, buffer_gcrypt, bufferlen);
	    gcry_cipher_close(encctx);
	}

	compare_buffers(buffer_gladman, buffer_gcrypt, bufferlen);

	{
	    gcry_cipher_hd_t decctx;
	    gcry_cipher_open(&decctx, GCRY_CIPHER_SERPENT256, GCRY_CIPHER_MODE_ECB, 0);
	    gcry_cipher_setkey(decctx, (uint8_t*)enckey, 32);
	    gcry_cipher_decrypt(decctx, buffer_gcrypt, bufferlen, buffer_gcrypt, bufferlen);
	    gcry_cipher_close(decctx);
	}

	check_buffer(buffer_gcrypt, bufferlen);
    }
#endif

#if HAVE_LIBMCRYPT
    { // libmcrypt

	char buffer_mcrypt[bufferlen];
	fill_buffer(buffer_mcrypt, bufferlen);

	{
	    MCRYPT encctx = mcrypt_module_open(MCRYPT_SERPENT, NULL, MCRYPT_ECB, NULL);
	    mcrypt_generic_init(encctx, enckey, 32, NULL);
	    mcrypt_generic(encctx, buffer_mcrypt, bufferlen);
	    mcrypt_generic_end(encctx);
	}

	compare_buffers(buffer_gladman, buffer_mcrypt, bufferlen);

	{
	    MCRYPT decctx = mcrypt_module_open(MCRYPT_SERPENT, NULL, MCRYPT_ECB, NULL);
	    mcrypt_generic_init(decctx, enckey, 32, NULL);
	    mdecrypt_generic(decctx, buffer_mcrypt, bufferlen);
	    mcrypt_generic_end(decctx);
	}

	check_buffer(buffer_mcrypt, bufferlen);
    }
#endif

#if HAVE_BOTAN
    { // Botan

	char buffer_botan[bufferlen];
	fill_buffer(buffer_botan, bufferlen);

	{
	    Botan::Serpent encctx;
	    encctx.set_key((Botan::byte*)enckey, 32);

	    for(unsigned int p = 0; p < bufferlen; p += encctx.BLOCK_SIZE)
		encctx.encrypt((Botan::byte*)buffer_botan + p);
	}

	compare_buffers(buffer_gladman, buffer_botan, bufferlen);

	{
	    Botan::Serpent decctx;
	    decctx.set_key((Botan::byte*)enckey, 32);

	    for(unsigned int p = 0; p < bufferlen; p += decctx.BLOCK_SIZE)
		decctx.decrypt((Botan::byte*)buffer_botan + p);
	}

	check_buffer(buffer_botan, bufferlen);
    }
#endif

#if HAVE_CRYPTOPP
    { // Crypto++

	uint8_t buffer_cryptopp[bufferlen];
	fill_buffer(buffer_cryptopp, bufferlen);

	{
	    CryptoPP::ECB_Mode<CryptoPP::Serpent>::Encryption encctx;
	    encctx.SetKey((uint8_t*)enckey, 32);

	    encctx.ProcessData(buffer_cryptopp, buffer_cryptopp, bufferlen);
	}

	compare_buffers(buffer_gladman, buffer_cryptopp, bufferlen);

	{
	    CryptoPP::ECB_Mode<CryptoPP::Serpent>::Decryption decctx;
	    decctx.SetKey((uint8_t*)enckey, 32);

	    decctx.ProcessData(buffer_cryptopp, buffer_cryptopp, bufferlen);
	}

	check_buffer(buffer_cryptopp, bufferlen);
    }
#endif

#if HAVE_LIBNETTLE
    { // Nettle

	uint8_t buffer_nettle[bufferlen];
	fill_buffer(buffer_nettle, bufferlen);

	{
	    serpent_ctx encctx;
	    serpent_set_key(&encctx, 32, (uint8_t*)enckey);
	    serpent_encrypt(&encctx, bufferlen, buffer_nettle, buffer_nettle);
	}

	// does not match!
	// compare_buffers(buffer_gladman, buffer_nettle, bufferlen);

	{
	    serpent_ctx decctx;
	    serpent_set_key(&decctx, 32, (uint8_t*)enckey);
	    serpent_decrypt(&decctx, bufferlen, buffer_nettle, buffer_nettle);
	}

	check_buffer(buffer_nettle, bufferlen);
    }
#endif

    // gladman implementation

    {
	SerpentGladman::DecryptECB decctx;

	decctx.set_key((uint8_t*)enckey, 256);
	decctx.decrypt(buffer_gladman, buffer_gladman, bufferlen);
    }

    check_buffer(buffer_gladman, bufferlen);
}

// *** Verify Twofish Implementations

void verify_twofish_ecb()
{
#if HAVE_LIBGCRYPT
    // libgcrypt

    char buffer_gcrypt[bufferlen];
    fill_buffer(buffer_gcrypt, bufferlen);

    {
	gcry_cipher_hd_t encctx;
	gcry_cipher_open(&encctx, GCRY_CIPHER_TWOFISH, GCRY_CIPHER_MODE_ECB, 0);
	gcry_cipher_setkey(encctx, (uint8_t*)enckey, 32);
	gcry_cipher_encrypt(encctx, buffer_gcrypt, bufferlen, buffer_gcrypt, bufferlen);
	gcry_cipher_close(encctx);
    }

#if HAVE_LIBMCRYPT
    { // libmcrypt

	char buffer_mcrypt[bufferlen];
	fill_buffer(buffer_mcrypt, bufferlen);

	{
	    MCRYPT encctx = mcrypt_module_open(MCRYPT_TWOFISH, NULL, MCRYPT_ECB, NULL);
	    mcrypt_generic_init(encctx, enckey, 32, NULL);
	    mcrypt_generic(encctx, buffer_mcrypt, bufferlen);
	    mcrypt_generic_end(encctx);
	}

	compare_buffers(buffer_gcrypt, buffer_mcrypt, bufferlen);

	{
	    MCRYPT decctx = mcrypt_module_open(MCRYPT_TWOFISH, NULL, MCRYPT_ECB, NULL);
	    mcrypt_generic_init(decctx, enckey, 32, NULL);
	    mdecrypt_generic(decctx, buffer_mcrypt, bufferlen);
	    mcrypt_generic_end(decctx);
	}

	check_buffer(buffer_mcrypt, bufferlen);
    }
#endif

#if HAVE_BOTAN
    { // Botan

	char buffer_botan[bufferlen];
	fill_buffer(buffer_botan, bufferlen);

	{
	    Botan::Twofish encctx;
	    encctx.set_key((Botan::byte*)enckey, 32);

	    for(unsigned int p = 0; p < bufferlen; p += encctx.BLOCK_SIZE)
		encctx.encrypt((Botan::byte*)buffer_botan + p);
	}

	compare_buffers(buffer_gcrypt, buffer_botan, bufferlen);

	{
	    Botan::Twofish decctx;
	    decctx.set_key((Botan::byte*)enckey, 32);

	    for(unsigned int p = 0; p < bufferlen; p += decctx.BLOCK_SIZE)
		decctx.decrypt((Botan::byte*)buffer_botan + p);
	}

	check_buffer(buffer_botan, bufferlen);
    }
#endif

#if HAVE_CRYPTOPP
    { // Crypto++

	char buffer_cryptopp[bufferlen];
	fill_buffer(buffer_cryptopp, bufferlen);

	{
	    CryptoPP::ECB_Mode<CryptoPP::Twofish>::Encryption encctx;
	    encctx.SetKey((uint8_t*)enckey, 32);

	    encctx.ProcessData((uint8_t*)buffer_cryptopp, (uint8_t*)buffer_cryptopp, bufferlen);
	}

	compare_buffers(buffer_gcrypt, buffer_cryptopp, bufferlen);

	{
	    CryptoPP::ECB_Mode<CryptoPP::Twofish>::Decryption decctx;
	    decctx.SetKey((uint8_t*)enckey, 32);

	    decctx.ProcessData((uint8_t*)buffer_cryptopp, (uint8_t*)buffer_cryptopp, bufferlen);
	}

	check_buffer(buffer_cryptopp, bufferlen);
    }
#endif

#if HAVE_LIBTOMCRYPT
    { // Tomcrypt

	char buffer_tomcrypt[bufferlen];
	fill_buffer(buffer_tomcrypt, bufferlen);

	{
	    symmetric_ECB encctx;
	    ecb_start(find_cipher("twofish"), (uint8_t*)enckey, 32, 0, &encctx);
	    ecb_encrypt((uint8_t*)buffer_tomcrypt, (uint8_t*)buffer_tomcrypt, bufferlen, &encctx);
	    ecb_done(&encctx);
	}

	compare_buffers(buffer_gcrypt, buffer_tomcrypt, bufferlen);

	{
	    symmetric_ECB decctx;
	    ecb_start(find_cipher("twofish"), (uint8_t*)enckey, 32, 0, &decctx);
	    ecb_decrypt((uint8_t*)buffer_tomcrypt, (uint8_t*)buffer_tomcrypt, bufferlen, &decctx);
	    ecb_done(&decctx);
	}

	check_buffer(buffer_tomcrypt, bufferlen);
    }
#endif

    {
	gcry_cipher_hd_t decctx;
	gcry_cipher_open(&decctx, GCRY_CIPHER_TWOFISH, GCRY_CIPHER_MODE_ECB, 0);
	gcry_cipher_setkey(decctx, (uint8_t*)enckey, 32);
	gcry_cipher_decrypt(decctx, buffer_gcrypt, bufferlen, buffer_gcrypt, bufferlen);
	gcry_cipher_close(decctx);
    }

    check_buffer(buffer_gcrypt, bufferlen);

#endif // HAVE_LIBGCRYPT
}

// *** Verify Triple DES Implementations

void verify_3des_ecb()
{
#if HAVE_LIBNETTLE
    // Nettle requires some parity fix of the key

    des_fix_parity(24, (uint8_t*)enckey, (uint8_t*)enckey);
#endif

#if HAVE_LIBGCRYPT
    // libgcrypt

    char buffer_gcrypt[bufferlen];
    fill_buffer(buffer_gcrypt, bufferlen);

    {
	gcry_cipher_hd_t encctx;
	gcry_cipher_open(&encctx, GCRY_CIPHER_3DES, GCRY_CIPHER_MODE_ECB, 0);
	gcry_cipher_setkey(encctx, (uint8_t*)enckey, 24);
	gcry_cipher_encrypt(encctx, buffer_gcrypt, bufferlen, buffer_gcrypt, bufferlen);
	gcry_cipher_close(encctx);
    }

#if HAVE_LIBMCRYPT
    { // libmcrypt

	char buffer_mcrypt[bufferlen];
	fill_buffer(buffer_mcrypt, bufferlen);

	{
	    MCRYPT encctx = mcrypt_module_open(MCRYPT_3DES, NULL, MCRYPT_ECB, NULL);
	    mcrypt_generic_init(encctx, enckey, 24, NULL);
	    mcrypt_generic(encctx, buffer_mcrypt, bufferlen);
	    mcrypt_generic_end(encctx);
	}

	compare_buffers(buffer_gcrypt, buffer_mcrypt, bufferlen);

	{
	    MCRYPT decctx = mcrypt_module_open(MCRYPT_3DES, NULL, MCRYPT_ECB, NULL);
	    mcrypt_generic_init(decctx, enckey, 24, NULL);
	    mdecrypt_generic(decctx, buffer_mcrypt, bufferlen);
	    mcrypt_generic_end(decctx);
	}

	check_buffer(buffer_mcrypt, bufferlen);
    }
#endif

#if HAVE_BOTAN
    { // Botan

	char buffer_botan[bufferlen];
	fill_buffer(buffer_botan, bufferlen);

	{
	    Botan::TripleDES encctx;
	    encctx.set_key((Botan::byte*)enckey, 24);

	    for(unsigned int p = 0; p < bufferlen; p += encctx.BLOCK_SIZE)
		encctx.encrypt((Botan::byte*)buffer_botan + p);
	}

	compare_buffers(buffer_gcrypt, buffer_botan, bufferlen);

	{
	    Botan::TripleDES decctx;
	    decctx.set_key((Botan::byte*)enckey, 24);

	    for(unsigned int p = 0; p < bufferlen; p += decctx.BLOCK_SIZE)
		decctx.decrypt((Botan::byte*)buffer_botan + p);
	}

	check_buffer(buffer_botan, bufferlen);
    }
#endif

#if HAVE_CRYPTOPP
    { // Crypto++

	char buffer_cryptopp[bufferlen];
	fill_buffer(buffer_cryptopp, bufferlen);

	{
	    CryptoPP::ECB_Mode<CryptoPP::DES_EDE3>::Encryption encctx;
	    encctx.SetKey((uint8_t*)enckey, 24);

	    encctx.ProcessData((uint8_t*)buffer_cryptopp, (uint8_t*)buffer_cryptopp, bufferlen);
	}

	compare_buffers(buffer_gcrypt, buffer_cryptopp, bufferlen);

	{
	    CryptoPP::ECB_Mode<CryptoPP::DES_EDE3>::Decryption decctx;
	    decctx.SetKey((uint8_t*)enckey, 24);

	    decctx.ProcessData((uint8_t*)buffer_cryptopp, (uint8_t*)buffer_cryptopp, bufferlen);
	}

	check_buffer(buffer_cryptopp, bufferlen);
    }
#endif

#if HAVE_OPENSSL
    { // OpenSSL

	char buffer_openssl[bufferlen];
	fill_buffer(buffer_openssl, bufferlen);

	{
	    DES_key_schedule eks1, eks2, eks3;

	    DES_set_key((DES_cblock*)(enckey +  0), &eks1);
	    DES_set_key((DES_cblock*)(enckey +  8), &eks2);
	    DES_set_key((DES_cblock*)(enckey + 16), &eks3);

	    for(unsigned int p = 0; p < bufferlen; p += 8)
		DES_encrypt3((DES_LONG*)(buffer_openssl + p), &eks1, &eks2, &eks3);
	}

	compare_buffers(buffer_gcrypt, buffer_openssl, bufferlen);

	{
	    DES_key_schedule dks1, dks2, dks3;

	    DES_set_key((DES_cblock*)(enckey +  0), &dks1);
	    DES_set_key((DES_cblock*)(enckey +  8), &dks2);
	    DES_set_key((DES_cblock*)(enckey + 16), &dks3);

	    for(unsigned int p = 0; p < bufferlen; p += 8)
		DES_decrypt3((DES_LONG*)(buffer_openssl + p), &dks1, &dks2, &dks3);
	}

	check_buffer(buffer_openssl, bufferlen);
    }
#endif

#if HAVE_LIBNETTLE
    { // Nettle

	char buffer_nettle[bufferlen];
	fill_buffer(buffer_nettle, bufferlen);

	{
	    des3_ctx encctx;
	    des3_set_key(&encctx, (uint8_t*)enckey);
	    des3_encrypt(&encctx, bufferlen, (uint8_t*)buffer_nettle, (uint8_t*)buffer_nettle);
	}

	compare_buffers(buffer_gcrypt, buffer_nettle, bufferlen);

	{
	    des3_ctx decctx;
	    des3_set_key(&decctx, (uint8_t*)enckey);
	    des3_decrypt(&decctx, bufferlen, (uint8_t*)buffer_nettle, (uint8_t*)buffer_nettle);
	}

	check_buffer(buffer_nettle, bufferlen);
    }
#endif

#if HAVE_LIBTOMCRYPT
    { // Tomcrypt

	char buffer_tomcrypt[bufferlen];
	fill_buffer(buffer_tomcrypt, bufferlen);

	{
	    symmetric_ECB encctx;
	    ecb_start(find_cipher("3des"), (uint8_t*)enckey, 24, 0, &encctx);
	    ecb_encrypt((uint8_t*)buffer_tomcrypt, (uint8_t*)buffer_tomcrypt, bufferlen, &encctx);
	    ecb_done(&encctx);
	}

	compare_buffers(buffer_gcrypt, buffer_tomcrypt, bufferlen);

	{
	    symmetric_ECB decctx;
	    ecb_start(find_cipher("3des"), (uint8_t*)enckey, 24, 0, &decctx);
	    ecb_decrypt((uint8_t*)buffer_tomcrypt, (uint8_t*)buffer_tomcrypt, bufferlen, &decctx);
	    ecb_done(&decctx);
	}

	check_buffer(buffer_tomcrypt, bufferlen);
    }
#endif

    {
	gcry_cipher_hd_t decctx;
	gcry_cipher_open(&decctx, GCRY_CIPHER_3DES, GCRY_CIPHER_MODE_ECB, 0);
	gcry_cipher_setkey(decctx, (uint8_t*)enckey, 24);
	gcry_cipher_decrypt(decctx, buffer_gcrypt, bufferlen, buffer_gcrypt, bufferlen);
	gcry_cipher_close(decctx);
    }

    check_buffer(buffer_gcrypt, bufferlen);

#endif // HAVE_LIBGCRYPT
}

int main()
{
    // Initialize all cryptographic libaries

#if HAVE_LIBGCRYPT
    gcry_check_version(GCRYPT_VERSION);
#endif

#if HAVE_BOTAN
    Botan::LibraryInitializer init;
#endif

#if HAVE_LIBTOMCRYPT
    register_cipher(&rijndael_desc);
    register_cipher(&twofish_desc);
    register_cipher(&des3_desc);
#endif

    // Create (somewhat) random encryption key

    srand(time(NULL));

    for(unsigned int i = 0; i < sizeof(enckey); ++i)
	enckey[i] = rand();

    // Verify cipher implementations

    verify_rijndael_ecb();
    verify_serpent_ecb();
    verify_twofish_ecb();
    verify_3des_ecb();

    return 0;
}
