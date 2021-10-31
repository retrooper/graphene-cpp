
// AesCfb128Encryptor.h

// Declares the cAesCfb128Encryptor class encrypting data using AES CFB-128





#pragma once

#include "mbedtls/aes.h"
#include <memory>




/** Encrypts data using the AES / CFB (128) algorithm */
class cAesCfb128Encryptor
{
public:

	cAesCfb128Encryptor();
	~cAesCfb128Encryptor();

	/** Initializes the decryptor with the specified Key / IV */
	void Init(const uint8_t a_Key[16], const uint8_t a_IV[16]);

	/** Encrypts a_Length bytes of the plain data; produces a_Length output bytes */
	void ProcessData(uint8_t * a_EncryptedOut, const uint8_t * a_PlainIn, size_t a_Length);

	/** Returns true if the object has been initialized with the Key / IV */
	bool IsValid() const { return m_IsValid; }

protected:
	mbedtls_aes_context m_Aes;

	/** The InitialVector, used by the CFB mode encryption */
	uint8_t m_IV[16];

	/** Indicates whether the object has been initialized with the Key / IV */
	bool m_IsValid;
} ;





