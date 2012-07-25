#pragma once

class BigNumber;

struct evp_cipher_ctx_st;
struct MD5state_st;
struct SHAstate_st;
struct hmac_ctx_st;
//////////////////////////////////////////////////////////////////////////

class SARC4
{
public:
	SARC4(int len);
	SARC4(BYTE *seed, int len);
	~SARC4();
	void Init(BYTE *seed);
	void UpdateData(int len, BYTE *data);
private:
	evp_cipher_ctx_st *m_pctx;
};

//////////////////////////////////////////////////////////////////////////

class Md5Hash
{
public:
	Md5Hash();
	~Md5Hash();

	void UpdateBigNumbers(BigNumber *bn0, ...);

	void UpdateData(const BYTE *dta, int len);
	void UpdateData(const std::string &str);

	void Initialize();
	void Finalize();

	BYTE *GetDigest(void) { return m_pDigest; };
	int GetLength(void);

private:
	MD5state_st *m_pst;
	BYTE *m_pDigest;
};

//////////////////////////////////////////////////////////////////////////

class Sha1Hash
{
public:
	Sha1Hash();
	~Sha1Hash();

	void UpdateBigNumbers(BigNumber *bn0, ...);

	void UpdateData(const BYTE *dta, int len);
	void UpdateData(const std::string &str);

	void Initialize();
	void Finalize();

	BYTE *GetDigest(void) { return m_pDigest; };
	int GetLength(void);

private:
	SHAstate_st *m_pst;
	BYTE *m_pDigest;
};

//////////////////////////////////////////////////////////////////////////

#define SEED_KEY_SIZE 16

class HMACSHA1
{
public:
	HMACSHA1(UINT len, BYTE *seed);
	~HMACSHA1();
	void UpdateBigNumber(BigNumber *bn);
	void UpdateData(const BYTE *data, int length);
	void UpdateData(const std::string &str);
	void Finalize();
	BYTE *ComputeHash(BigNumber *bn);
	BYTE *GetDigest() { return m_pDigest; }
	int GetLength();
private:
	hmac_ctx_st *m_pctx;
	BYTE *m_pDigest;
};

//////////////////////////////////////////////////////////////////////////

class CAuthServerCrypt
{
public:
	CAuthServerCrypt();
	~CAuthServerCrypt();

	void Init(BigNumber *K);
	void DecryptRecv(BYTE *, size_t);
	void EncryptSend(BYTE *, size_t);

	bool IsInitialized() { return m_initialized; }

private:
	SARC4 m_clientDecrypt;
	SARC4 m_serverEncrypt;
	bool m_initialized;
};

//////////////////////////////////////////////////////////////////////////

class CAuthClientCrypt
{
public:
	CAuthClientCrypt();
	~CAuthClientCrypt();

	void Init(BigNumber *K);
	void DecryptRecv(BYTE *, size_t);
	void EncryptSend(BYTE *, size_t);

	bool IsInitialized() { return m_initialized; }

private:
	SARC4 m_clientEncrypt;
	SARC4 m_serverDecrypt;
	bool m_initialized;
};