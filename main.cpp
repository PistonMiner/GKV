#include "gkv.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	GKV w;
	w.show();
	return a.exec();
}
