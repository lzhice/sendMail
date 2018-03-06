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
	m_sMailInfo.m_pcUserName = "这里输入你的邮箱账号"; 
	m_sMailInfo.m_pcUserPassWord = "这里输入你的授权码";
	m_sMailInfo.m_pcSenderName = "昵称";
	m_sMailInfo.m_pcSender = "";//发送者的邮箱地址    
	m_sMailInfo.m_pcReceiver = "673041955@qq.com";//接收者的邮箱地址    
	m_sMailInfo.m_pcTitle = "yqsheng邮件";//邮箱标题    
	m_sMailInfo.m_pcBody = "今天我吃了饭，想打人";//邮件文本正文    
	m_sMailInfo.m_pcIPAddr = "";//服务器的IP可以留空，二选一，对于QT来说一个结果  
	m_sMailInfo.m_pcIPName = "smtp.163.com";//服务器的名称（IP与名称二选一，优先取名称）   
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
	// 分配给pBuff64  ↑ 分配给pBuff64+1  
	//         point所在的位置  
	static char Base64Encode[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";//base64所映射的字符表  
	int point;//每一个源字符拆分的位置，可取2,4,6；初始为2  
	point = 2;
	int i;
	int iIndex;//base64字符的索引  
	char n = 0;//上一个源字符的残留值  
	for (i = 0; i < iLen; i++)
	{
		if (point == 2)
		{
			iIndex = ((*pSrcBuff) >> point) & 0x3f;//取得pSrcBuff的高point位  
		}
		else if (point == 4)
		{
			iIndex = ((*pSrcBuff) >> point) & 0xf;//取得pSrcBuff的高point位  
		}
		else if (point == 6)
		{
			iIndex = ((*pSrcBuff) >> point) & 0x3;//取得pSrcBuff的高point位  
		}
		iIndex += n;//与pSrcBuff-1的低point结合组成Base64的索引  
		*pBuff64++ = Base64Encode[iIndex];//由索引表得到pBuff64  
		n = ((*pSrcBuff) << (6 - point));//计算源字符中的残留值  
		n = n & 0x3f;//确保n的最高两位为0  
		point += 2;//源字符的拆分位置上升2  
		if (point == 8)//如果拆分位置为8说明pSrcBuff有6位残留，可以组成一个完整的Base64字符，所以直接再组合一次  
		{
			iIndex = (*pSrcBuff) & 0x3f;//提取低6位，这就是索引了  
			*pBuff64++ = Base64Encode[iIndex];//  
			n = 0;//残留值为0  
			point = 2;//拆分位置设为2  
		}
		pSrcBuff++;

	}
	if (n != 0)
	{
		*pBuff64++ = Base64Encode[n];
	}
	if (iLen % 3 == 2)//如果源字符串长度不是3的倍数要用'='补全  
	{
		*pBuff64 = '=';
	}
	else if (iLen % 3 == 1)
	{
		*pBuff64++ = '=';
		*pBuff64 = '=';
	}
}

void CSendMail::AddFilePath(char * pcFilePath)//添加附件路径  
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
		if (strcmp(pcFilePath, temp) == 0)//如果已经存在就不用再添加了  
		{
			return;
		}
	}
	m_pcFilePathList.push_back(pcFilePath);
}

void CSendMail::DeleteFilePath(char* pcFilePath)//删除附件路径  
{
	int i;
	char* temp;
	for (i = 0; i < m_pcFilePathList.count(); i++)
	{
		temp = m_pcFilePathList.at(i);
		if (strcmp(temp, pcFilePath) == 0)//找到并删除它，如果没找到就算了  
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
	client->connectToHost(m_sMailInfo.m_pcIPName, 25);//使用25端口，可以不用ssl连接
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
	client->write(m_cSendBuff, strlen(m_cSendBuff)); //请求登录
	client->waitForBytesWritten();
	ui.textEdit->append(m_cSendBuff);
	client->waitForReadyRead(9000);


	//m_cReceiveBuff 334

	memset(m_cSendBuff, 0, sizeof(m_cSendBuff));
	Char2Base64(m_cSendBuff, m_sMailInfo.m_pcUserName, strlen(m_sMailInfo.m_pcUserName));
	m_cSendBuff[strlen(m_cSendBuff)] = '\r';
	m_cSendBuff[strlen(m_cSendBuff)] = '\n';
	client->write(m_cSendBuff, strlen(m_cSendBuff));//发送用户名  
	client->waitForBytesWritten();

	ui.textEdit->append(m_cSendBuff);
	client->waitForReadyRead(9000);

	//m_cReceiveBuff 334

	memset(m_cSendBuff, 0, sizeof(m_cSendBuff));
	Char2Base64(m_cSendBuff, m_sMailInfo.m_pcUserPassWord, strlen(m_sMailInfo.m_pcUserPassWord));
	m_cSendBuff[strlen(m_cSendBuff)] = '\r';
	m_cSendBuff[strlen(m_cSendBuff)] = '\n';
	client->write(m_cSendBuff, strlen(m_cSendBuff));//发送用户密码  
	client->waitForBytesWritten();
	ui.textEdit->append(m_cSendBuff);
	client->waitForReadyRead(9000);
	
	//m_cReceiveBuff 235
	return true;//登录成功  
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
	if (!Logon())//登录邮箱  
	{
		QMessageBox::information(NULL, "Title", "Logon", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
		client->close();
		return false;
	}
	if (!SendHead())//发送邮件头  
	{
		QMessageBox::information(NULL, "Title", "SendHead", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
		client->close();
		return false;
	}
	if (!SendTextBody())//发送邮件文本部分  
	{
		QMessageBox::information(NULL, "Title", "SendTextBody", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
		client->close();
		return false;
	}
	if (!SendEnd())//结束邮件，并关闭sock  
	{
		QMessageBox::information(NULL, "Title", "SendEnd", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
		client->close();
		return false;
	}

	return true;
}
