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
				std::cout<<"�޷����ӵ�������"<<std::endl;
			}
		}
		catch(std::wstring strErr)
		{
			std::cout<<"����"<<UnicodeToMB(strErr)<<std::endl;
		}
	}
public:
	//����������Ϣ
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
	//�����ȡ��Ϣ
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
					std::cout<<"��¼�ɹ���"<<std::endl;
					break;
				case RET_FAIL_UNKNOWN_ACCOUNT:
					std::cout<<"�û��������ڣ�"<<std::endl;
					break;
				}
				break;
			}
		}
		return true;
	}
	//����ر���Ϣ
	virtual bool OnSocketClose(CTcpSocket * pTcpSocke, bool bCloseByServer)
	{
		if(bCloseByServer)
		{
			std::cout<<"�������رգ�"<<std::endl;
		}
		else
		{
			std::cout<<"�ͻ���ǿ���˳���"<<std::endl;
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