#include "sendMail.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	CSendMail w;
	w.show();
	return a.exec();
}
