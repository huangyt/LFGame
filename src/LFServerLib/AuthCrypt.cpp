#include "GlobalDef.h"
#include "AuthCrypt.h"
#include "BigNumber.h"

#include "openssl/evp.h"
#include "openssl/sha.h"
#include "openssl/md5.h"
#include "openssl/crypto.h"
#include "openssl/hmac.h"

SARC4::SARC4(int len)
{
	m_pctx = EVP_CIPHER_CTX_new();

	EVP_CIPHER_CTX_init(m_pctx);
	EVP_EncryptInit_ex(m_pctx, EVP_rc4(), NULL, NULL, NULL);
	EVP_CIPHER_CTX_set_key_length(m_pctx, len);
}

SARC4::SARC4(BYTE *seed, int len)
{
	m_pctx = EVP_CIPHER_CTX_new();

	EVP_CIPHER_CTX_init(m_pctx);
	EVP_EncryptInit_ex(m_pctx, EVP_rc4(), NULL, NULL, NULL);
	EVP_CIPHER_CTX_set_key_length(m_pctx, len);
	EVP_EncryptInit_ex(m_pctx, NULL, NULL, seed, NULL);
}

SARC4::~SARC4()
{
	EVP_CIPHER_CTX_cleanup(m_pctx);

	EVP_CIPHER_CTX_free(m_pctx);
}

void SARC4::Init(BYTE *seed)
{
	EVP_EncryptInit_ex(m_pctx, NULL, NULL, seed, NULL);
}

void SARC4::UpdateData(int len, BYTE *data)
{
	int outlen = 0;
	EVP_EncryptUpdate(m_pctx, data, &outlen, data, len);
	EVP_EncryptFinal_ex(m_pctx, data, &outlen);
}

//////////////////////////////////////////////////////////////////////////

Md5Hash::Md5Hash()
{
	m_pDigest = new BYTE[MD5_DIGEST_LENGTH];
	m_pst = new MD5state_st;

	MD5_Init(m_pst);
}

Md5Hash::~Md5Hash()
{
	MD5_Init(m_pst);

	SAFE_DELETE(m_pst);
	SafeDeleteArray(m_pDigest);
}

void Md5Hash::UpdateData(const BYTE *dta, int len)
{
	MD5_Update(m_pst, dta, len);
}

void Md5Hash::UpdateData(const std::string &str)
{
	UpdateData((BYTE const*)str.c_str(), str.length());
}

void Md5Hash::UpdateBigNumbers(BigNumber *bn0, ...)
{
	va_list v;
	BigNumber *bn;

	va_start(v, bn0);
	bn = bn0;
	while (bn)
	{
		UpdateData(bn->AsByteArray(), bn->GetNumBytes());
		bn = va_arg(v, BigNumber *);
	}
	va_end(v);
}

void Md5Hash::Initialize()
{
	MD5_Init(m_pst);
}

void Md5Hash::Finalize()
{
	MD5_Final(m_pDigest, m_pst);
}

int Md5Hash::GetLength()
{
	return MD5_DIGEST_LENGTH;
}

//////////////////////////////////////////////////////////////////////////

Sha1Hash::Sha1Hash()
{
	m_pDigest = new BYTE[SHA_DIGEST_LENGTH];
	m_pst = new SHAstate_st;

	SHA1_Init(m_pst);
}

Sha1Hash::~Sha1Hash()
{
	SHA1_Init(m_pst);

	SAFE_DELETE(m_pst);
	SafeDeleteArray(m_pDigest);
}

void Sha1Hash::UpdateData(const BYTE *dta, int len)
{
	SHA1_Update(m_pst, dta, len);
}

void Sha1Hash::UpdateData(const std::string &str)
{
	UpdateData((BYTE const*)str.c_str(), str.length());
}

void Sha1Hash::UpdateBigNumbers(BigNumber *bn0, ...)
{
	va_list v;
	BigNumber *bn;

	va_start(v, bn0);
	bn = bn0;
	while (bn)
	{
		UpdateData(bn->AsByteArray(), bn->GetNumBytes());
		bn = va_arg(v, BigNumber *);
	}
	va_end(v);
}

void Sha1Hash::Initialize()
{
	SHA1_Init(m_pst);
}

void Sha1Hash::Finalize(void)
{
	SHA1_Final(m_pDigest, m_pst);
}

int Sha1Hash::GetLength()
{
	return SHA_DIGEST_LENGTH;
}

//////////////////////////////////////////////////////////////////////////

HMACSHA1::HMACSHA1(UINT len, BYTE *seed)
{
	m_pDigest = new BYTE[SHA_DIGEST_LENGTH];
	m_pctx = new hmac_ctx_st;

	HMAC_CTX_init(m_pctx);
	HMAC_Init_ex(m_pctx, seed, len, EVP_sha1(), NULL);
}

HMACSHA1::~HMACSHA1()
{
	HMAC_CTX_cleanup(m_pctx);

	SAFE_DELETE(m_pctx);
	SafeDeleteArray(m_pDigest);
}

void HMACSHA1::UpdateBigNumber(BigNumber *bn)
{
	UpdateData(bn->AsByteArray(), bn->GetNumBytes());
}

void HMACSHA1::UpdateData(const BYTE *data, int length)
{
	HMAC_Update(m_pctx, data, length);
}

void HMACSHA1::UpdateData(const std::string &str)
{
	UpdateData((BYTE const*)str.c_str(), str.length());
}

void HMACSHA1::Finalize()
{
	UINT length = 0;
	HMAC_Final(m_pctx, m_pDigest, &length);
	ASSERT(length == SHA_DIGEST_LENGTH);
}

BYTE *HMACSHA1::ComputeHash(BigNumber *bn)
{
	HMAC_Update(m_pctx, bn->AsByteArray(), bn->GetNumBytes());
	Finalize();
	return m_pDigest;;
}

int HMACSHA1::GetLength()
{
	return SHA_DIGEST_LENGTH;
}

//////////////////////////////////////////////////////////////////////////

CAuthServerCrypt::CAuthServerCrypt() 
	: m_clientDecrypt(SHA_DIGEST_LENGTH)
	, m_serverEncrypt(SHA_DIGEST_LENGTH)
{
	m_initialized = false;
}

CAuthServerCrypt::~CAuthServerCrypt()
{

}

void CAuthServerCrypt::Init(BigNumber *K)
{
	BYTE ServerEncryptionKey[SEED_KEY_SIZE] = { 0xCC, 0x98, 0xAE, 0x04, 0xE8, 0x97, 0xEA, 0xCA, 0x12, 0xDD, 0xC0, 0x93, 0x42, 0x91, 0x53, 0x57 };

	HMACSHA1 serverEncryptHmac(SEED_KEY_SIZE, ServerEncryptionKey);
	BYTE *encryptHash = serverEncryptHmac.ComputeHash(K);

	BYTE ServerDecryptionKey[SEED_KEY_SIZE] = { 0xC2, 0xB3, 0x72, 0x3C, 0xC6, 0xAE, 0xD9, 0xB5, 0x34, 0x3C, 0x53, 0xEE, 0x2F, 0x43, 0x67, 0xCE };

	HMACSHA1 clientDecryptHmac(SEED_KEY_SIZE, ServerDecryptionKey);
	BYTE *decryptHash = clientDecryptHmac.ComputeHash(K);

	m_clientDecrypt.Init(decryptHash);
	m_serverEncrypt.Init(encryptHash);

	BYTE syncBuf[1024];

	memset(syncBuf, 0, 1024);
	m_serverEncrypt.UpdateData(1024, syncBuf);

	memset(syncBuf, 0, 1024);
	m_clientDecrypt.UpdateData(1024, syncBuf);

	m_initialized = true;
}

void CAuthServerCrypt::DecryptRecv(BYTE *data, size_t len)
{
	if (!m_initialized)
		return;

	m_clientDecrypt.UpdateData(len, data);
}

void CAuthServerCrypt::EncryptSend(BYTE *data, size_t len)
{
	if (!m_initialized)
		return;

	m_serverEncrypt.UpdateData(len, data);
}

// //////////////////////////////////////////////////////////////////////////
 
CAuthClientCrypt::CAuthClientCrypt() 
	: m_clientEncrypt(SHA_DIGEST_LENGTH)
	, m_serverDecrypt(SHA_DIGEST_LENGTH)
{
	m_initialized = false;
}

CAuthClientCrypt::~CAuthClientCrypt()
{

}

void CAuthClientCrypt::Init(BigNumber *K)
{
	BYTE ClientDecryptionKey[SEED_KEY_SIZE] = { 0xCC, 0x98, 0xAE, 0x04, 0xE8, 0x97, 0xEA, 0xCA, 0x12, 0xDD, 0xC0, 0x93, 0x42, 0x91, 0x53, 0x57 };

	HMACSHA1 serverEncryptHmac(SEED_KEY_SIZE, ClientDecryptionKey);
	BYTE *decryptHash = serverEncryptHmac.ComputeHash(K);

	BYTE ClientEncryptionKey[SEED_KEY_SIZE] = { 0xC2, 0xB3, 0x72, 0x3C, 0xC6, 0xAE, 0xD9, 0xB5, 0x34, 0x3C, 0x53, 0xEE, 0x2F, 0x43, 0x67, 0xCE };

	HMACSHA1 clientDecryptHmac(SEED_KEY_SIZE, ClientEncryptionKey);
	BYTE *encryptHash = clientDecryptHmac.ComputeHash(K);

	m_clientEncrypt.Init(encryptHash);
	m_serverDecrypt.Init(decryptHash);

	BYTE syncBuf[1024];

	memset(syncBuf, 0, 1024);
	m_clientEncrypt.UpdateData(1024, syncBuf);

	memset(syncBuf, 0, 1024);
	m_serverDecrypt.UpdateData(1024, syncBuf);

	m_initialized = true;
}

void CAuthClientCrypt::DecryptRecv(BYTE *data, size_t len)
{
	if (!m_initialized)
		return;

	m_serverDecrypt.UpdateData(len, data);
}

void CAuthClientCrypt::EncryptSend(BYTE *data, size_t len)
{
	if (!m_initialized)
		return;

	m_clientEncrypt.UpdateData(len, data);
}