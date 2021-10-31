
// RsaPrivateKey.h

// Declares the cRsaPrivateKey class representing a private key for RSA operations.





#pragma once

#include "CtrDrbgContext.h"
#include "mbedtls/rsa.h"
#include <vector>
#include <iostream>



/** Encapsulates an RSA private key used in PKI cryptography */
class cRsaPrivateKey
{
	friend class cSslContext;

public:
	/** Creates a new empty object, the key is not assigned */
	cRsaPrivateKey(void);

	/** Deep-copies the key from a_Other */
	cRsaPrivateKey(const cRsaPrivateKey & a_Other);

	~cRsaPrivateKey();

	/** Generates a new key within this object, with the specified size in bits.
	Returns true on success, false on failure. */
	bool Generate(unsigned a_KeySizeBits = 1024);

	/** Returns the public key part encoded in ASN1 DER encoding */
	std::vector<char> GetPubKeyDER();

	/** Decrypts the data using RSAES-PKCS#1 algorithm.
	Both a_EncryptedData and a_DecryptedData must be at least <KeySizeBytes> bytes large.
	Returns the number of bytes decrypted, or negative number for error. */
	int Decrypt(std::vector<uint8_t> a_EncryptedData, uint8_t * a_DecryptedData, size_t a_DecryptedMaxLength);

protected:
	/** The mbedTLS key context */
	mbedtls_rsa_context m_Rsa;

	/** The random generator used for generating the key and encryption / decryption */
	cCtrDrbgContext m_CtrDrbg;


	/** Returns the internal context ptr. Only use in mbedTLS API calls. */
	mbedtls_rsa_context * GetInternal(void) { return &m_Rsa; }
} ;

typedef std::shared_ptr<cRsaPrivateKey> cRsaPrivateKeyPtr;





