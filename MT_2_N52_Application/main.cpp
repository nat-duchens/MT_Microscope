#include "MT_2_N52.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MT_2_N52 w;
	w.setFixedSize(w.size());
	w.show();
	return a.exec();
}
