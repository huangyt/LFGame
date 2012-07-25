#pragma once

struct bignum_st;

class BigNumber
{
public:
	BigNumber();
	BigNumber(const BigNumber &bn);
	BigNumber(UINT);
	~BigNumber();

	void SetDword(UINT);
	void SetQword(UINT64);
	void SetBinary(const BYTE *bytes, int len);
	void SetHexStr(const char *str);

	void SetRand(int numbits);

	BigNumber operator=(const BigNumber &bn);

	BigNumber operator+=(const BigNumber &bn);
	BigNumber operator+(const BigNumber &bn)
	{
		BigNumber t(*this);
		return t += bn;
	}
	BigNumber operator-=(const BigNumber &bn);
	BigNumber operator-(const BigNumber &bn)
	{
		BigNumber t(*this);
		return t -= bn;
	}
	BigNumber operator*=(const BigNumber &bn);
	BigNumber operator*(const BigNumber &bn)
	{
		BigNumber t(*this);
		return t *= bn;
	}
	BigNumber operator/=(const BigNumber &bn);
	BigNumber operator/(const BigNumber &bn)
	{
		BigNumber t(*this);
		return t /= bn;
	}
	BigNumber operator%=(const BigNumber &bn);
	BigNumber operator%(const BigNumber &bn)
	{
		BigNumber t(*this);
		return t %= bn;
	}

	bool isZero() const;

	BigNumber ModExp(const BigNumber &bn1, const BigNumber &bn2);
	BigNumber Exp(const BigNumber &);

	int GetNumBytes(void);

	struct bignum_st *BN() { return _bn; }

	UINT AsDword();
	BYTE* AsByteArray(int minSize = 0, bool reverse = true);

	const char *AsHexStr();
	const char *AsDecStr();

private:
	struct bignum_st *_bn;
	BYTE *_array;
};