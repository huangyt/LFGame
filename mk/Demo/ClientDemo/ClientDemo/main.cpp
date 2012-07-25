#include "TcpSocket.h"
#include "CMDDefine.h"
#include "ByteBuffer.h"
#include "FormatString.h"
#include <signal.h>
#include <iostream>

class Test : public CTcpSocketSink
{
public:
	Test(){}
	~Test(){}
public:
	void Connect()
	{
		m_Socket.SetSocketSink(static_cast<CTcpSocketSink*>(this));
		m_Socket.CloseSocket(false);

		try
		{
			if(m_Socket.Connect(L"127.0.0.1", 6700)==false)
			{
				std::cout<<"无法连接到服务器"<<std::endl;
			}
		}
		catch(std::wstring strErr)
		{
			std::cout<<"错误："<<UnicodeToMB(strErr)<<std::endl;
		}
	}
public:
	//网络连接消息
	virtual bool OnSocketConnect(int iErrorCode, const std::wstring& strErrorDesc, CTcpSocket * pTcpSocke)
	{
		if(iErrorCode==0)
		{
			pTcpSocke->SetNoDelayMod(true);

			std::string strUserName = "lifeng";
			BYTE buffer[100] = {0};
			sLFLogonChanllenge *cl = (sLFLogonChanllenge*)buffer;
			cl->size = sizeof(sLFLogonChanllenge) + strUserName.size();
			cl->version1 = 0;
			cl->version2 = 0;
			cl->version3 = 1;
			cl->build = 120303;
			cl->I_len = strUserName.size() + 1;
			strncpy_s((char*)cl->I, strUserName.size()+1, strUserName.c_str(), strUserName.size());

			pTcpSocke->SendData(CMD_LF_LOGON_CHALLENGE, cl, cl->size);
		}
		else
		{
			std::cout<<UnicodeToMB(strErrorDesc)<<std::endl;
		}
		return true;
	}
	//网络读取消息
	virtual bool OnSocketRead(DWORD dwCommand, void * pBuffer, WORD wDataSize, CTcpSocket * pTcpSocke)
	{
		m_RecvBuffer.resize(wDataSize);
		if(NULL != pBuffer && 0 != wDataSize)
		{
			m_RecvBuffer.put(0, static_cast<BYTE*>(pBuffer), wDataSize);
		}
		switch(dwCommand)
		{
		case CMD_LF_LOGON_CHALLENGE:
			{
				DWORD dwRet = 0;
				m_RecvBuffer>>dwRet;
				switch(dwRet)
				{
				case RET_SUCCESS:
					std::cout<<"登录成功！"<<std::endl;
					break;
				case RET_FAIL_UNKNOWN_ACCOUNT:
					std::cout<<"用户名不存在！"<<std::endl;
					break;
				}
				break;
			}
		}
		return true;
	}
	//网络关闭消息
	virtual bool OnSocketClose(CTcpSocket * pTcpSocke, bool bCloseByServer)
	{
		if(bCloseByServer)
		{
			std::cout<<"服务器关闭！"<<std::endl;
		}
		else
		{
			std::cout<<"客户端强制退出！"<<std::endl;
		}
		return true;
	}
	void close()
	{
		m_Socket.CloseSocket(true);
	}
private:
	CTcpSocket	m_Socket;
	CByteBuffer m_RecvBuffer;
};

bool stopEvent = false;

void UnhookSignals();
void HookSignals();

int main()
{
	Test a;
	a.Connect();

	HookSignals();

	while(!stopEvent)
	{
		Sleep(1);
	}

	a.close();

	UnhookSignals();
}

void OnSignal(int s)
{
	switch (s)
	{
	case SIGINT:
	case SIGTERM:
	case SIGBREAK:
		stopEvent = true;
		Sleep(1000);
		break;
	}

	signal(s, OnSignal);
}

void HookSignals()
{
	signal(SIGINT, OnSignal);
	signal(SIGTERM, OnSignal);
	signal(SIGBREAK, OnSignal);
}

void UnhookSignals()
{
	signal(SIGINT, 0);
	signal(SIGTERM, 0);
	signal(SIGBREAK, 0);
}