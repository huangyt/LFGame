#include "LFGui.h"
#include "LFGuiElement.h"

namespace LFG
{
#define CHECK_KEY_EXIST(type)	PropertyMapIter iter = m_PropertyMap.find(strKey); \
								if(iter == m_PropertyMap.end()) \
									return FALSE; \
								PROPERTY_NODE &node = iter->second; \
								if(node._type != type) \
									return FALSE;

	CLFGuiElement::CLFGuiElement()
	{
		this->RegisterProperty(L"type", PROPERTY_STRING);
		this->RegisterProperty(L"name", PROPERTY_STRING);
	}

	CLFGuiElement::~CLFGuiElement()
	{
		for(PropertyMapIter iter = m_PropertyMap.begin();
			iter != m_PropertyMap.end(); ++iter)
		{
			SAFE_DELETE(iter->second._value);
		}
		m_PropertyMap.clear();
	}

	BOOL CLFGuiElement::SetProperty(const std::wstring &strKey, const int &nValue)
	{
		CHECK_KEY_EXIST(PROPERTY_INT);

		int* pValue = static_cast<int*>(node._value);
		*pValue = nValue;

		return TRUE;
	}

	BOOL CLFGuiElement::SetProperty(const std::wstring &strKey, const UINT &nValue)
	{
		CHECK_KEY_EXIST(PROPERTY_UINT);

		UINT* pValue = static_cast<UINT*>(node._value);
		*pValue = nValue;

		return TRUE;
	}

	BOOL CLFGuiElement::SetProperty(const std::wstring &strKey, const float &fValue)
	{
		CHECK_KEY_EXIST(PROPERTY_FLOAT);

		float* pValue = static_cast<float*>(node._value);
		*pValue = fValue;

		return TRUE;
	}

	BOOL CLFGuiElement::SetProperty(const std::wstring &strKey, const std::wstring &strValue)
	{
		CHECK_KEY_EXIST(PROPERTY_STRING);

		std::wstring* pValue = static_cast<std::wstring*>(node._value);
		*pValue = strValue;

		return TRUE;
	}

	BOOL CLFGuiElement::GetProperty(const std::wstring &strKey, int &nValue)
	{
		CHECK_KEY_EXIST(PROPERTY_INT);

		nValue = *static_cast<int*>(node._value);

		return TRUE;
	}

	BOOL CLFGuiElement::GetProperty(const std::wstring &strKey, UINT &nValue)
	{
		CHECK_KEY_EXIST(PROPERTY_UINT);

		nValue = *static_cast<UINT*>(node._value);

		return TRUE;
	}

	BOOL CLFGuiElement::GetProperty(const std::wstring &strKey, float &fValue)
	{
		CHECK_KEY_EXIST(PROPERTY_FLOAT);

		fValue = *static_cast<float*>(node._value);

		return TRUE;
	}

	BOOL CLFGuiElement::GetProperty(const std::wstring &strKey, std::wstring &strValue)
	{
		CHECK_KEY_EXIST(PROPERTY_STRING);

		strValue = *static_cast<std::wstring*>(node._value);

		return TRUE;
	}

	BOOL CLFGuiElement::RegisterProperty(const std::wstring &strKey, const PROPERTY_TYPE &type)
	{
		PropertyMapIter iter = m_PropertyMap.find(strKey);
		if(iter != m_PropertyMap.end())
		{
			return FALSE;
		}
		PROPERTY_NODE node;
		node._type = type;
		switch(type)
		{
		case PROPERTY_INT:
			node._value = new int(0);
			break;
		case PROPERTY_UINT:
			node._value = new UINT(0);
			break;
		case PROPERTY_FLOAT:
			node._value = new float(0.0f);
			break;
		case PROPERTY_STRING:
			node._value = new std::wstring;
			break;
		default:
			node._value = NULL;
			break;
		}
		m_PropertyMap.insert(PropertyMap::value_type(strKey, node));

		return TRUE;
	}
}