
// AesCfb128Encryptor.cpp

// Implements the cAesCfb128Encryptor class encrypting data using AES CFB-128

#include "AesCfb128Encryptor.h"





cAesCfb128Encryptor::cAesCfb128Encryptor():
	m_IsValid(false)
{
	mbedtls_aes_init(&m_Aes);
}





cAesCfb128Encryptor::~cAesCfb128Encryptor()
{
	// Clear the leftover in-memory data, so that they can't be accessed by a backdoor
	mbedtls_aes_free(&m_Aes);
}





void cAesCfb128Encryptor::Init(const uint8_t a_Key[16], const uint8_t a_IV[16])
{
	memcpy(m_IV, a_IV, 16);
	mbedtls_aes_setkey_enc(&m_Aes, a_Key, 128);
	m_IsValid = true;
}





void cAesCfb128Encryptor::ProcessData(uint8_t * const a_EncryptedOut, const uint8_t * const a_PlainIn, size_t a_Length)
{
	mbedtls_aes_crypt_cfb8(&m_Aes, MBEDTLS_AES_ENCRYPT, a_Length, m_IV, reinterpret_cast<const unsigned char *>(a_PlainIn), reinterpret_cast<unsigned char *>(a_EncryptedOut));
}
