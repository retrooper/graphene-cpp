
// RsaPrivateKey.cpp

#include "RsaPrivateKey.h"
#include "mbedtls/pk.h"


#define MBEDTLS_RSA_PRIVATE 1


cRsaPrivateKey::cRsaPrivateKey(void)
{
	mbedtls_rsa_init(&m_Rsa, MBEDTLS_RSA_PKCS_V15, 0);
	m_CtrDrbg.Initialize("RSA", 3);
}





cRsaPrivateKey::cRsaPrivateKey(const cRsaPrivateKey & a_Other)
{
	mbedtls_rsa_init(&m_Rsa, MBEDTLS_RSA_PKCS_V15, 0);
	mbedtls_rsa_copy(&m_Rsa, &a_Other.m_Rsa);
	m_CtrDrbg.Initialize("RSA", 3);
}





cRsaPrivateKey::~cRsaPrivateKey()
{
	mbedtls_rsa_free(&m_Rsa);
}





bool cRsaPrivateKey::Generate(unsigned a_KeySizeBits)
{
	int res = mbedtls_rsa_gen_key(&m_Rsa, mbedtls_ctr_drbg_random, m_CtrDrbg.GetInternal(), a_KeySizeBits, 65537);
	if (res != 0)
	{
		return false;
	}

	return true;
}





std::vector<char> cRsaPrivateKey::GetPubKeyDER(void)
{
	class cPubKey
	{
	public:
		cPubKey(mbedtls_rsa_context * a_Rsa) :
			m_IsValid(false)
		{
			mbedtls_pk_init(&m_Key);
			if (mbedtls_pk_setup(&m_Key, mbedtls_pk_info_from_type(MBEDTLS_PK_RSA)) != 0)
			{
				std::cerr << "Failed to init private key!" << std::endl;
				return {};
			}
			if (mbedtls_rsa_copy(mbedtls_pk_rsa(m_Key), a_Rsa) != 0)
			{
                std::cerr << "Failed to init private key to pk context!" << std::endl;
				return {};
			}
			m_IsValid = true;
		}

		~cPubKey()
		{
			if (m_IsValid)
			{
				mbedtls_pk_free(&m_Key);
			}
		}

		operator mbedtls_pk_context * () { return &m_Key; }

	protected:
		bool m_IsValid;
		mbedtls_pk_context m_Key;
	} PkCtx(&m_Rsa);

	unsigned char buf[3000];
	int res = mbedtls_pk_write_pubkey_der(PkCtx, buf, sizeof(buf));
	if (res < 0)
	{
		return {};
	}
    std::vector<char> publicKeyBytes(res);
    for (int i = 0; i < res; i++) {
        publicKeyBytes.push_back(buf[i]);
    }
	return publicKeyBytes;
}





int cRsaPrivateKey::Decrypt(const std::vector<uint8_t> a_EncryptedData, char* a_DecryptedData, size_t a_DecryptedMaxLength)
{
	if (a_EncryptedData.size() < m_Rsa.private_len)
	{
		return -1;
	}
    //TODO Check if private_len works, otherwise use mbedtls_rsa_get_len(&m_Rsa)
	if (a_DecryptedMaxLength < m_Rsa.private_len)
	{
        return -1;
	}
	size_t DecryptedLength;
	int res = mbedtls_rsa_pkcs1_decrypt(
		&m_Rsa, mbedtls_ctr_drbg_random, m_CtrDrbg.GetInternal(), MBEDTLS_RSA_PRIVATE, &DecryptedLength,
		reinterpret_cast<const unsigned char *>(a_EncryptedData.data()), a_DecryptedData, a_DecryptedMaxLength
	);
	if (res != 0)
	{
		return -1;
	}
	return static_cast<int>(DecryptedLength);
}
