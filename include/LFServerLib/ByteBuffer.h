#pragma once

#include "GlobalDef.h"

template<class T>
struct Unused
{
	Unused() {}
};

class CByteBuffer
{
public:
	const static size_t DEFAULT_SIZE = 0x1000;

	CByteBuffer(): m_rpos(0), m_wpos(0)
	{
		m_Storage.reserve(DEFAULT_SIZE);
	}

	CByteBuffer(size_t res): m_rpos(0), m_wpos(0)
	{
		m_Storage.reserve(res);
	}

	CByteBuffer(const CByteBuffer &buf): m_rpos(buf.m_rpos), m_wpos(buf.m_wpos), m_Storage(buf.m_Storage) { }

	void clear()
	{
		m_Storage.clear();
		m_rpos=m_wpos=0;
	}

	template <typename T> void put(size_t pos,T value)
	{
		put(pos,(BYTE *)&value,sizeof(value));
	}

	CByteBuffer &operator<<(BYTE value)
	{
		append<BYTE>(value);
		return *this;
	}

	CByteBuffer &operator<<(WORD value)
	{
		append<WORD>(value);
		return *this;
	}

	CByteBuffer &operator<<(DWORD value)
	{
		append<UINT>(value);
		return *this;
	}

	CByteBuffer &operator<<(UINT64 value)
	{
		append<UINT64>(value);
		return *this;
	}

	CByteBuffer &operator<<(char value)
	{
		append<char>(value);
		return *this;
	}

	CByteBuffer &operator<<(short value)
	{
		append<short>(value);
		return *this;
	}

	CByteBuffer &operator<<(int value)
	{
		append<int>(value);
		return *this;
	}

	CByteBuffer &operator<<(INT64 value)
	{
		append<INT64>(value);
		return *this;
	}

	CByteBuffer &operator<<(float value)
	{
		append<float>(value);
		return *this;
	}

	CByteBuffer &operator<<(double value)
	{
		append<double>(value);
		return *this;
	}

	CByteBuffer &operator<<(const std::string &value)
	{
		append((BYTE const *)value.c_str(), value.length());
		append((BYTE)0);
		return *this;
	}

	CByteBuffer &operator<<(const char *str)
	{
		append((BYTE const *)str, str ? strlen(str) : 0);
		append((BYTE)0);
		return *this;
	}

	CByteBuffer &operator>>(bool &value)
	{
		value = read<char>() > 0 ? true : false;
		return *this;
	}

	CByteBuffer &operator>>(BYTE &value)
	{
		value = read<BYTE>();
		return *this;
	}

	CByteBuffer &operator>>(WORD &value)
	{
		value = read<WORD>();
		return *this;
	}

	CByteBuffer &operator>>(DWORD &value)
	{
		value = read<DWORD>();
		return *this;
	}

	CByteBuffer &operator>>(UINT64 &value)
	{
		value = read<UINT64>();
		return *this;
	}

	CByteBuffer &operator>>(char &value)
	{
		value = read<char>();
		return *this;
	}

	CByteBuffer &operator>>(short &value)
	{
		value = read<short>();
		return *this;
	}

	CByteBuffer &operator>>(int &value)
	{
		value = read<int>();
		return *this;
	}

	CByteBuffer &operator>>(INT64 &value)
	{
		value = read<INT64>();
		return *this;
	}

	CByteBuffer &operator>>(float &value)
	{
		value = read<float>();
		return *this;
	}

	CByteBuffer &operator>>(double &value)
	{
		value = read<double>();
		return *this;
	}

	CByteBuffer &operator>>(std::string& value)
	{
		value.clear();
		while (rpos() < size())
		{
			char c = read<char>();
			if (c == 0)
				break;
			value += c;
		}
		return *this;
	}

	template<class T>
	CByteBuffer &operator>>(Unused<T> const&)
	{
		read_skip<T>();
		return *this;
	}

	BYTE operator[](size_t pos) const
	{
		return read<BYTE>(pos);
	}

	size_t rpos() const { return m_rpos; }

	size_t rpos(size_t rpos_)
	{
		m_rpos = rpos_;
		return m_rpos;
	}

	size_t wpos() const { return m_wpos; }

	size_t wpos(size_t wpos_)
	{
		m_wpos = wpos_;
		return m_wpos;
	}

	template<typename T>
	void read_skip() { read_skip(sizeof(T)); }

	void read_skip(size_t skip)
	{
		if(m_rpos + skip > size())
			return;
		m_rpos += skip;
	}

	template <typename T> T read()
	{
		T r = read<T>(m_rpos);
		m_rpos += sizeof(T);
		return r;
	}

	template <typename T> T read(size_t pos) const
	{
		if(pos + sizeof(T) > size())
			return 0;
		T val = *((T const*)&m_Storage[pos]);
		return val;
	}

	void read(BYTE *dest, size_t len)
	{
		if(m_rpos  + len > size())
			return;
		memcpy(dest, &m_Storage[m_rpos], len);
		m_rpos += len;
	}

	UINT64 readPackGUID()
	{
		UINT64 guid = 0;
		BYTE guidmark = 0;
		(*this) >> guidmark;

		for(int i = 0; i < 8; ++i)
		{
			if(guidmark & (BYTE(1) << i))
			{
				BYTE bit;
				(*this) >> bit;
				guid |= (UINT64(bit) << (i * 8));
			}
		}

		return guid;
	}

	const BYTE *contents() const { return &m_Storage[0]; }

	size_t size() const { return m_Storage.size(); }
	bool empty() const { return m_Storage.empty(); }

	void resize(size_t newsize)
	{
		m_Storage.resize(newsize);
		m_rpos = 0;
		m_wpos = size();
	}

	void reserve(size_t ressize)
	{
		if (ressize > size())
			m_Storage.reserve(ressize);
	}

	void append(const std::string& str)
	{
		append((BYTE const*)str.c_str(), str.size() + 1);
	}

	void append(const char *src, size_t cnt)
	{
		return append((const BYTE *)src, cnt);
	}

	template<class T> void append(const T *src, size_t cnt)
	{
		return append((const uint8 *)src, cnt * sizeof(T));
	}

	void append(const BYTE *src, size_t cnt)
	{
		if (!cnt)
			return;

		ASSERT(size() < SOCKET_BUFFER);

		if (m_Storage.size() < m_wpos + cnt)
			m_Storage.resize(m_wpos + cnt);
		memcpy(&m_Storage[m_wpos], src, cnt);
		m_wpos += cnt;
	}

	void append(const CByteBuffer& buffer)
	{
		if(buffer.wpos())
			append(buffer.contents(), buffer.wpos());
	}

	void appendPackXYZ(float x, float y, float z)
	{
		BYTE packed = 0;
		packed |= ((int)(x / 0.25f) & 0x7FF);
		packed |= ((int)(y / 0.25f) & 0x7FF) << 11;
		packed |= ((int)(z / 0.25f) & 0x3FF) << 22;
		*this << packed;
	}

	void appendPackGUID(UINT64 guid)
	{
		BYTE packGUID[8+1];
		packGUID[0] = 0;
		size_t size = 1;
		for (BYTE i = 0; guid != 0; ++i)
		{
			if (guid & 0xFF)
			{
				packGUID[0] |= BYTE(1 << i);
				packGUID[size] =  BYTE(guid & 0xFF);
				++size;
			}

			guid >>= 8;
		}

		append(packGUID, size);
	}

	bool put(size_t pos, const BYTE *src, size_t cnt)
	{
		if(pos + cnt > size())
			false;
		memcpy(&m_Storage[pos], src, cnt);
		return true;
	}

private:
	template <typename T> void append(T value)
	{
		append((BYTE *)&value, sizeof(value));
	}

protected:
	size_t				m_rpos;
	size_t				m_wpos;
	std::vector<BYTE>	m_Storage;
};

template <typename T>
inline CByteBuffer &operator<<(CByteBuffer &b, std::vector<T> const& v)
{
	b << (UINT)v.size();
	for (typename std::vector<T>::iterator i = v.begin(); i != v.end(); ++i)
	{
		b << *i;
	}
	return b;
}

template <typename T>
inline CByteBuffer &operator>>(CByteBuffer &b, std::vector<T> &v)
{
	UINT vsize;
	b >> vsize;
	v.clear();
	while(vsize--)
	{
		T t;
		b >> t;
		v.push_back(t);
	}
	return b;
}

template <typename T>
inline CByteBuffer &operator<<(CByteBuffer &b, std::unordered_set<T> const& v)
{
	b << (UINT)v.size();
	for (typename std::list<T>::iterator i = v.begin(); i != v.end(); ++i)
	{
		b << *i;
	}
	return b;
}

template <typename T>
inline CByteBuffer &operator>>(CByteBuffer &b, std::unordered_set<T> &v)
{
	UINT vsize;
	b >> vsize;
	v.clear();
	while(vsize--)
	{
		T t;
		b >> t;
		v.insert(t);
	}
	return b;
}

template <typename K, typename V>
inline CByteBuffer &operator<<(CByteBuffer &b, std::unordered_map<K, V> &m)
{
	b << (UINT)m.size();
	for (typename std::map<K, V>::iterator i = m.begin(); i != m.end(); ++i)
	{
		b << i->first << i->second;
	}
	return b;
}

template <typename K, typename V>
inline CByteBuffer &operator>>(CByteBuffer &b, std::unordered_map<K, V> &m)
{
	UINT msize;
	b >> msize;
	m.clear();
	while(msize--)
	{
		K k;
		V v;
		b >> k >> v;
		m.insert(std::make_pair(k, v));
	}
	return b;
}

template<>
inline void CByteBuffer::read_skip<char*>()
{
	std::string temp;
	*this >> temp;
}

template<>
inline void CByteBuffer::read_skip<char const*>()
{
	read_skip<char*>();
}

template<>
inline void CByteBuffer::read_skip<std::string>()
{
	read_skip<char*>();
}