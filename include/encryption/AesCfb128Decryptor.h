
// AesCfb128Decryptor.h

// Declares the cAesCfb128Decryptor class decrypting data using AES CFB-128





#pragma once

#if PLATFORM_CRYPTOGRAPHY && defined(_WIN32)
#include <wincrypt.h>
#else
#include "mbedtls/aes.h"
#endif
#include <algorithm>






/** Decrypts data using the AES / CFB 128 algorithm */
class cAesCfb128Decryptor
{
public:

	cAesCfb128Decryptor(void);
	~cAesCfb128Decryptor();

	/** Initializes the decryptor with the specified Key / IV */
	void Init(const uint8_t a_Key[16], const uint8_t a_IV[16]);

	/** Decrypts a_Length bytes of the encrypted data in-place; produces a_Length output bytes */
	void ProcessData(std::byte * a_EncryptedIn, size_t a_Length);

	/** Returns true if the object has been initialized with the Key / IV */
	bool IsValid() const { return m_IsValid; }

protected:

#if PLATFORM_CRYPTOGRAPHY && defined(_WIN32)
	HCRYPTPROV m_Aes;
	HCRYPTKEY m_Key;
#else
	mbedtls_aes_context m_Aes;
#endif

	/** The InitialVector, used by the CFB mode decryption */
	uint8_t m_IV[16];

	/** Indicates whether the object has been initialized with the Key / IV */
	bool m_IsValid;
} ;
