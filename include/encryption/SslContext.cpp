
// SslContext.cpp

// Implements the cSslContext class that holds everything a single SSL context needs to function

#include "SslContext.h"
#include "SslConfig.h"





cSslContext::cSslContext() :
	m_IsValid(false),
	m_HasHandshaken(false)
{
	mbedtls_ssl_init(&m_Ssl);
}





cSslContext::~cSslContext()
{
	mbedtls_ssl_free(&m_Ssl);
}





int cSslContext::Initialize(std::shared_ptr<const cSslConfig> a_Config)
{
	// Check double-initialization:
	if (m_IsValid)
	{
		return MBEDTLS_ERR_SSL_BAD_INPUT_DATA;  // There is no return value well-suited for this, reuse this one.
	}

	// Check the Config:
	m_Config = std::move(a_Config);
	if (m_Config == nullptr)
	{
		return MBEDTLS_ERR_SSL_BAD_INPUT_DATA;
	}

	// Apply the configuration to the ssl context
	int res = mbedtls_ssl_setup(&m_Ssl, m_Config->GetInternal());
	if (res != 0)
	{
		return res;
	}

	// Set the io callbacks
	mbedtls_ssl_set_bio(&m_Ssl, this, SendEncrypted, ReceiveEncrypted, nullptr);

	m_IsValid = true;
	return 0;
}





int cSslContext::Initialize(bool a_IsClient)
{
	if (a_IsClient)
	{
		return Initialize(cSslConfig::GetDefaultClientConfig());
	}
	else
	{
		return Initialize(cSslConfig::GetDefaultServerConfig());
	}
}





void cSslContext::SetExpectedPeerName(const std::string & a_ExpectedPeerName)
{
	mbedtls_ssl_set_hostname(&m_Ssl, a_ExpectedPeerName.c_str());
}





int cSslContext::WritePlain(const void * a_Data, size_t a_NumBytes)
{
	if (!m_HasHandshaken)
	{
		int res = Handshake();
		if (res != 0)
		{
			return res;
		}
	}

	return mbedtls_ssl_write(&m_Ssl, static_cast<const unsigned char *>(a_Data), a_NumBytes);
}





int cSslContext::ReadPlain(void * a_Data, size_t a_MaxBytes)
{
	if (!m_HasHandshaken)
	{
		int res = Handshake();
		if (res != 0)
		{
			return res;
		}
	}

	return mbedtls_ssl_read(&m_Ssl, static_cast<unsigned char *>(a_Data), a_MaxBytes);
}





int cSslContext::Handshake()
{
	int res = mbedtls_ssl_handshake(&m_Ssl);
	if (res == 0)
	{
		m_HasHandshaken = true;
	}
	return res;
}





int cSslContext::NotifyClose()
{
	return mbedtls_ssl_close_notify(&m_Ssl);
}




