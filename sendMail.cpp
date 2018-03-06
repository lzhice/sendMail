#include "sendMail.h"
#include <QMessageBox>

CSendMail::CSendMail(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	client = new QTcpSocket(this);
	connect(ui.test_send, SIGNAL(clicked()), this, SLOT(CReateSocket()));
	connect(client, SIGNAL(readyRead()), this, SLOT(readMessage()), Qt::DirectConnection);
	connect(client, SIGNAL(connected()), this, SLOT(SendMail()));
	m_pcFileBuff = NULL;
	memset(m_cSendBuff, 0, sizeof(m_cSendBuff));
	memset(m_cReceiveBuff, 0, sizeof(m_cReceiveBuff));
	m_sMailInfo.m_pcUserName = "����������������˺�"; 
	m_sMailInfo.m_pcUserPassWord = "�������������Ȩ��";
	m_sMailInfo.m_pcSenderName = "�ǳ�";
	m_sMailInfo.m_pcSender = "";//�����ߵ������ַ    
	m_sMailInfo.m_pcReceiver = "673041955@qq.com";//�����ߵ������ַ    
	m_sMailInfo.m_pcTitle = "yqsheng�ʼ�";//�������    
	m_sMailInfo.m_pcBody = "�����ҳ��˷��������";//�ʼ��ı�����    
	m_sMailInfo.m_pcIPAddr = "";//��������IP�������գ���ѡһ������QT��˵һ�����  
	m_sMailInfo.m_pcIPName = "smtp.163.com";//�����������ƣ�IP�����ƶ�ѡһ������ȡ���ƣ�   
}
CSendMail::~CSendMail()
{
	DeleteAllPath();
}
void CSendMail::readMessage()
{
	memset(m_cReceiveBuff, '\0', 1024);
	int ret = client->read(m_cReceiveBuff, 1024);
	ui.textEdit->append(m_cReceiveBuff);
}
void CSendMail::Char2Base64(char* pBuff64, char* pSrcBuff, int iLen)
{
	//1   1   1   1   1   1   1   1  
	// �����pBuff64  �� �����pBuff64+1  
	//         point���ڵ�λ��  
	static char Base64Encode[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";//base64��ӳ����ַ���  
	int point;//ÿһ��Դ�ַ���ֵ�λ�ã���ȡ2,4,6����ʼΪ2  
	point = 2;
	int i;
	int iIndex;//base64�ַ�������  
	char n = 0;//��һ��Դ�ַ��Ĳ���ֵ  
	for (i = 0; i < iLen; i++)
	{
		if (point == 2)
		{
			iIndex = ((*pSrcBuff) >> point) & 0x3f;//ȡ��pSrcBuff�ĸ�pointλ  
		}
		else if (point == 4)
		{
			iIndex = ((*pSrcBuff) >> point) & 0xf;//ȡ��pSrcBuff�ĸ�pointλ  
		}
		else if (point == 6)
		{
			iIndex = ((*pSrcBuff) >> point) & 0x3;//ȡ��pSrcBuff�ĸ�pointλ  
		}
		iIndex += n;//��pSrcBuff-1�ĵ�point������Base64������  
		*pBuff64++ = Base64Encode[iIndex];//��������õ�pBuff64  
		n = ((*pSrcBuff) << (6 - point));//����Դ�ַ��еĲ���ֵ  
		n = n & 0x3f;//ȷ��n�������λΪ0  
		point += 2;//Դ�ַ��Ĳ��λ������2  
		if (point == 8)//������λ��Ϊ8˵��pSrcBuff��6λ�������������һ��������Base64�ַ�������ֱ�������һ��  
		{
			iIndex = (*pSrcBuff) & 0x3f;//��ȡ��6λ�������������  
			*pBuff64++ = Base64Encode[iIndex];//  
			n = 0;//����ֵΪ0  
			point = 2;//���λ����Ϊ2  
		}
		pSrcBuff++;

	}
	if (n != 0)
	{
		*pBuff64++ = Base64Encode[n];
	}
	if (iLen % 3 == 2)//���Դ�ַ������Ȳ���3�ı���Ҫ��'='��ȫ  
	{
		*pBuff64 = '=';
	}
	else if (iLen % 3 == 1)
	{
		*pBuff64++ = '=';
		*pBuff64 = '=';
	}
}

void CSendMail::AddFilePath(char * pcFilePath)//��Ӹ���·��  
{
	if (pcFilePath == NULL)
	{
		return;
	}
	int i;
	char* temp;
	for (i = 0; i < m_pcFilePathList.count(); i++)
	{
		temp = m_pcFilePathList.at(i);
		if (strcmp(pcFilePath, temp) == 0)//����Ѿ����ھͲ����������  
		{
			return;
		}
	}
	m_pcFilePathList.push_back(pcFilePath);
}

void CSendMail::DeleteFilePath(char* pcFilePath)//ɾ������·��  
{
	int i;
	char* temp;
	for (i = 0; i < m_pcFilePathList.count(); i++)
	{
		temp = m_pcFilePathList.at(i);
		if (strcmp(temp, pcFilePath) == 0)//�ҵ���ɾ���������û�ҵ�������  
		{
			m_pcFilePathList.removeAt(i);
			delete[] temp;
			return;
		}
	}
}


void CSendMail::DeleteAllPath(void)
{
	m_pcFilePathList.clear();
}

int CSendMail::GetFileData(char* FilePath)
{
	m_pcFileBuff = NULL;
	if (FilePath == NULL)
	{
		return 0;
	}
	QFile f(FilePath);
	int len;


	if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		return 0;
	}
	len = (int)f.size();
	m_pcFileBuff = new char[len + 1];
	memset(m_pcFileBuff, 0, len + 1);
	f.read(m_pcFileBuff, len);

	f.close();
	return len;
}

void CSendMail::GetFileName(char* fileName, char* filePath)
{
	if (filePath == NULL || fileName == NULL)
	{
		return;
	}
	int i;
	for (i = 0; i < (int)strlen(filePath); i++)
	{
		if (filePath[strlen(filePath) - 1 - i] == '\\')
		{
			memcpy(fileName, &filePath[strlen(filePath) - i], i);
			return;
		}
	}
}

bool CSendMail::CReateSocket()
{
	client->connectToHost(m_sMailInfo.m_pcIPName, 25);//ʹ��25�˿ڣ����Բ���ssl����
	return true;
}


bool CSendMail::Logon()
{

	memset(m_cSendBuff, 0, sizeof(m_cSendBuff));
	sprintf_s(m_cSendBuff, "HELO []\r\n");
	
	client->write(m_cSendBuff, strlen(m_cSendBuff));
	client->waitForBytesWritten();
	ui.textEdit->append(m_cSendBuff);
	client->waitForReadyRead(9000);
		
	//m_cReceiveBuff 250 

	
	memset(m_cSendBuff, 0, sizeof(m_cSendBuff));
	sprintf_s(m_cSendBuff, "AUTH LOGIN\r\n");
	client->write(m_cSendBuff, strlen(m_cSendBuff)); //�����¼
	client->waitForBytesWritten();
	ui.textEdit->append(m_cSendBuff);
	client->waitForReadyRead(9000);


	//m_cReceiveBuff 334

	memset(m_cSendBuff, 0, sizeof(m_cSendBuff));
	Char2Base64(m_cSendBuff, m_sMailInfo.m_pcUserName, strlen(m_sMailInfo.m_pcUserName));
	m_cSendBuff[strlen(m_cSendBuff)] = '\r';
	m_cSendBuff[strlen(m_cSendBuff)] = '\n';
	client->write(m_cSendBuff, strlen(m_cSendBuff));//�����û���  
	client->waitForBytesWritten();

	ui.textEdit->append(m_cSendBuff);
	client->waitForReadyRead(9000);

	//m_cReceiveBuff 334

	memset(m_cSendBuff, 0, sizeof(m_cSendBuff));
	Char2Base64(m_cSendBuff, m_sMailInfo.m_pcUserPassWord, strlen(m_sMailInfo.m_pcUserPassWord));
	m_cSendBuff[strlen(m_cSendBuff)] = '\r';
	m_cSendBuff[strlen(m_cSendBuff)] = '\n';
	client->write(m_cSendBuff, strlen(m_cSendBuff));//�����û�����  
	client->waitForBytesWritten();
	ui.textEdit->append(m_cSendBuff);
	client->waitForReadyRead(9000);
	
	//m_cReceiveBuff 235
	return true;//��¼�ɹ�  
}


bool CSendMail::SendHead()
{
	int rt;
	memset(m_cSendBuff, '\0', sizeof(m_cSendBuff));
	sprintf_s(m_cSendBuff, "MAIL FROM:<%s>\r\n", m_sMailInfo.m_pcSender);
	rt = client->write(m_cSendBuff, strlen(m_cSendBuff));
	client->waitForBytesWritten();
	client->waitForReadyRead(9000);
	if (rt != strlen(m_cSendBuff))
	{
		return false;
	}


	memset(m_cSendBuff, '\0', sizeof(m_cSendBuff));
	sprintf_s(m_cSendBuff, "RCPT TO:<%s>\r\n", m_sMailInfo.m_pcReceiver);
	rt = client->write(m_cSendBuff, strlen(m_cSendBuff));
	client->waitForBytesWritten();
	client->waitForReadyRead(9000);
	if (rt != strlen(m_cSendBuff))
	{
		return false;
	}
	
	memset(m_cSendBuff, '\0', sizeof(m_cSendBuff));
	memcpy(m_cSendBuff, "DATA\r\n", strlen("DATA\r\n"));
	rt = client->write(m_cSendBuff, strlen(m_cSendBuff));
	client->waitForBytesWritten();
	client->waitForReadyRead(9000);
	if (rt != strlen(m_cSendBuff))
	{
		return false;
	}

	memset(m_cSendBuff, '\0', sizeof(m_cSendBuff));
	sprintf_s(m_cSendBuff, "From:\"%s\"<%s>\r\n", m_sMailInfo.m_pcSenderName, m_sMailInfo.m_pcSender);
	sprintf_s(&m_cSendBuff[strlen(m_cSendBuff)], 150, "To:\"INVT.COM.CN\"<%s>\r\n", m_sMailInfo.m_pcReceiver);
	sprintf_s(&m_cSendBuff[strlen(m_cSendBuff)], 150, "Subject:%s\r\nMime-Version: 1.0\r\nContent-Type: multipart/mixed; boundary=\"INVT\"\r\n\r\n", m_sMailInfo.m_pcTitle);
	rt = client->write(m_cSendBuff, strlen(m_cSendBuff));
	client->waitForBytesWritten();
	client->waitForReadyRead(9000);
	if (rt != strlen(m_cSendBuff))
	{
		return false;
	}

	return true;
}

bool CSendMail::SendTextBody()
{
	int rt=0;
	memset(m_cSendBuff, 0, sizeof(m_cSendBuff));
	sprintf_s(m_cSendBuff, "--INVT\r\nContent-Type: text/plain;\r\n  charset=\"gb2312\"\r\n\r\n%s\r\n\r\n", m_sMailInfo.m_pcBody);
	rt = client->write(m_cSendBuff, strlen(m_cSendBuff));
	client->waitForBytesWritten();
	client->waitForReadyRead(9000);
	if (rt != strlen(m_cSendBuff))
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool CSendMail::SendFileBody()
{
	int i;
	char* filePath;
	int rt=0;
	int len=0;
	int pt = 0;
	char fileName[128];
	for (i = 0; i < m_pcFilePathList.count(); i++)
	{
		pt = 0;
		memset(fileName, 0, 128);
		filePath = m_pcFilePathList.at(i);
		len = GetFileData(filePath);
		GetFileName(fileName, filePath);

		sprintf_s(m_cSendBuff, "--INVT\r\nContent-Type: application/octet-stream;\r\n  name=\"%s\"\r\nContent-Transfer-Encoding: base64\r\nContent-Disposition: attachment;\r\n  filename=\"%s\"\r\n\r\n", fileName, fileName);
		client->write(m_cSendBuff, strlen(m_cSendBuff)+1);
		client->waitForBytesWritten();
		client->waitForReadyRead(9000);
		while (pt < len)
		{
			memset(m_cSendBuff, '\0', sizeof(m_cSendBuff));
			Char2Base64(m_cSendBuff, &m_pcFileBuff[pt], std::min(len - pt, 3000));
			m_cSendBuff[strlen(m_cSendBuff)] = '\r';
			m_cSendBuff[strlen(m_cSendBuff)] = '\n';
			rt = client->write(m_cSendBuff, strlen(m_cSendBuff)+1);
			client->waitForBytesWritten();
			client->waitForReadyRead(9000);
			pt += std::min(len - pt, 3000);
			if (rt != strlen(m_cSendBuff))
			{
				return false;
			}
		}
		if (len != 0)
		{
			delete[] m_pcFileBuff;
		}
	}

	return true;
}

bool CSendMail::SendEnd()
{
	sprintf_s(m_cSendBuff, "--INVT--\r\n.\r\n");
	client->write(m_cSendBuff, strlen(m_cSendBuff) + 1);

	client->waitForBytesWritten();
	client->waitForReadyRead(9000);

	sprintf_s(m_cSendBuff, "QUIT\r\n");
	client->write(m_cSendBuff, strlen(m_cSendBuff) + 1);
	client->waitForBytesWritten();
	client->waitForReadyRead(9000);
	client->close();

	return true;
}


bool CSendMail::SendMail()
{
	
	if (m_sMailInfo.m_pcBody == NULL
		|| m_sMailInfo.m_pcIPAddr == NULL
		|| m_sMailInfo.m_pcIPName == NULL
		|| m_sMailInfo.m_pcReceiver == NULL
		|| m_sMailInfo.m_pcSender == NULL
		|| m_sMailInfo.m_pcSenderName == NULL
		|| m_sMailInfo.m_pcTitle == NULL
		|| m_sMailInfo.m_pcUserName == NULL
		|| m_sMailInfo.m_pcUserPassWord == NULL)
	{
		return false;
	}
	if (!Logon())//��¼����  
	{
		QMessageBox::information(NULL, "Title", "Logon", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
		client->close();
		return false;
	}
	if (!SendHead())//�����ʼ�ͷ  
	{
		QMessageBox::information(NULL, "Title", "SendHead", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
		client->close();
		return false;
	}
	if (!SendTextBody())//�����ʼ��ı�����  
	{
		QMessageBox::information(NULL, "Title", "SendTextBody", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
		client->close();
		return false;
	}
	if (!SendEnd())//�����ʼ������ر�sock  
	{
		QMessageBox::information(NULL, "Title", "SendEnd", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
		client->close();
		return false;
	}

	return true;
}
