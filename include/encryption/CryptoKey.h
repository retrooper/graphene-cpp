
// CryptoKey.h

// Declares the cCryptoKey class representing a RSA public key in mbedTLS





#pragma once

#include "CtrDrbgContext.h"
#include "mbedtls/pk.h"
#include <string>
#include <memory>


class cCryptoKey
{
	friend class cSslConfig;

public:
	/** Constructs an empty key instance. Before use, it needs to be filled by ParsePublic() or ParsePrivate() */
	cCryptoKey();

	/** Constructs the public key out of the DER- or PEM-encoded pubkey data */
	cCryptoKey(const std::string & a_PublicKeyData);

	/** Constructs the private key out of the DER- or PEM-encoded privkey data, with the specified password.
	If a_Password is empty, no password is assumed. */
	cCryptoKey(const std::string & a_PrivateKeyData, const std::string & a_Password);

	~cCryptoKey();

	/** Decrypts the data using the stored public key
	Both a_EncryptedData and a_DecryptedData must be at least <KeySizeBytes> bytes large.
	Returns the number of bytes decrypted, or negative number for error. */
	int Decrypt(const uint8_t * a_EncryptedData, size_t a_EncryptedLength, uint8_t * a_DecryptedData, size_t a_DecryptedMaxLength);

	/** Encrypts the data using the stored public key
	Both a_EncryptedData and a_DecryptedData must be at least <KeySizeBytes> bytes large.
	Returns the number of bytes decrypted, or negative number for error. */
	int Encrypt(const uint8_t * a_PlainData, size_t a_PlainLength, uint8_t * a_EncryptedData, size_t a_EncryptedMaxLength);

	/** Parses the specified data into a public key representation.
	The key can be DER- or PEM-encoded.
	Returns 0 on success, mbedTLS error code on failure. */
	int ParsePublic(const void * a_Data, size_t a_NumBytes);

	/** Parses the specified data into a private key representation.
	If a_Password is empty, no password is assumed.
	The key can be DER- or PEM-encoded.
	Returns 0 on success, mbedTLS error code on failure. */
	int ParsePrivate(const void * a_Data, size_t a_NumBytes, const std::string & a_Password);

	/** Returns true if the contained key is valid. */
	bool IsValid() const;

protected:
	/** The mbedTLS representation of the key data */
	mbedtls_pk_context m_Pk;

	/** The random generator used in encryption and decryption */
	cCtrDrbgContext m_CtrDrbg;


	/** Returns the internal context ptr. Only use in mbedTLS API calls. */
	mbedtls_pk_context * GetInternal(void) { return &m_Pk; }
} ;

typedef std::shared_ptr<cCryptoKey> cCryptoKeyPtr;




