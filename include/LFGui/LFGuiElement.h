#pragma once

namespace LFG
{
	enum PROPERTY_TYPE
	{
		PROPERTY_INT	= 0,
		PROPERTY_UINT	= 1,
		PROPERTY_FLOAT	= 2,
		PROPERTY_STRING = 3
	};

	//非线程安全
	class CLFGuiElement
	{
	public:
		BOOL SetProperty(const std::wstring &strKey, const int &nValue);
		BOOL SetProperty(const std::wstring &strKey, const UINT &nValue);
		BOOL SetProperty(const std::wstring &strKey, const float &fValue);
		BOOL SetProperty(const std::wstring &strKey, const std::wstring &strValue);

		BOOL GetProperty(const std::wstring &strKey, int &nValue);
		BOOL GetProperty(const std::wstring &strKey, UINT &nValue);
		BOOL GetProperty(const std::wstring &strKey, float &fValue);
		BOOL GetProperty(const std::wstring &strKey, std::wstring &strValue);

		BOOL RegisterProperty(const std::wstring &strKey, const PROPERTY_TYPE &type);

	protected:
		CLFGuiElement();
		virtual ~CLFGuiElement();

	private:
		struct PROPERTY_NODE
		{
			PROPERTY_TYPE _type;
			void* _value;
		};
		typedef std::unordered_map<std::wstring, PROPERTY_NODE> PropertyMap;
		typedef PropertyMap::iterator PropertyMapIter;
		PropertyMap m_PropertyMap;
	};
}