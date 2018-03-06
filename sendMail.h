#pragma once

#include <QtWidgets/QWidget>
#include "ui_sendMail.h"
#include <QList>
#include <QFile>
#include <QtNetwork>
#include <algorithm>
#include <QTcpSocket>
struct sMailInfo //�ʼ���Ϣ    
{
	char*   m_pcUserName;//�û���¼���������    
	char*   m_pcUserPassWord;//�û���¼���������    
	char*   m_pcSenderName;//�û�����ʱ��ʾ������    
	char*   m_pcSender;//�����ߵ������ַ    
	char*   m_pcReceiver;//�����ߵ������ַ    
	char*   m_pcTitle;//�������    
	char*   m_pcBody;//�ʼ��ı�����    
	char*   m_pcIPAddr;//��������IP    
	char*   m_pcIPName;//�����������ƣ�IP�����ƶ�ѡһ������ȡ���ƣ�    
	sMailInfo() { memset(this, 0, sizeof(sMailInfo)); }
};
class CSendMail : public QWidget
{
	Q_OBJECT

public:
	CSendMail(QWidget *parent = Q_NULLPTR);
	~CSendMail();
private:
	Ui::sendMailClass ui;
private slots:
	bool SendMail();//�����ʼ�����Ҫ�ڷ��͵�ʱ���ʼ���ʼ���Ϣ  
	void readMessage();	
	bool  CReateSocket();//����socket����    
public:
	  
	void AddFilePath(char * pcFilePath);//��Ӹ����ľ���·���������б���    
	void DeleteFilePath(char* pcFilePath);//ɾ������·��������еĻ�    
	void DeleteAllPath(void);//ɾ��ȫ��������·��    

protected:
	void GetFileName(char* fileName, char* filePath);//�Ӹ�����·���л�ȡ�ļ�����    
	void Char2Base64(char* pBuff64, char* pSrcBuff, int iLen);//��char����ת����Base64����    
	
	bool Logon();//��¼���䣬��Ҫ���з��ʼ�ǰ��׼������    
	int GetFileData(char* FilePath);//���ļ�·����ȡ��������    

	bool SendHead();//�����ʼ�ͷ    
	bool SendTextBody();//�����ʼ��ı�����    
	bool SendFileBody();//�����ʼ�����    
	bool SendEnd();//�����ʼ���β    
	
protected:
	QList<char*> m_pcFilePathList;//��¼����·��    
	QTcpSocket *client;
	char  m_cSendBuff[4096];//���ͻ�����    
	char  m_cReceiveBuff[1024];
	char* m_pcFileBuff;//ָ�򸽼�����    
	struct sMailInfo m_sMailInfo;

};
