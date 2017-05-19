#include "labelv2.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	a.setApplicationName("LabelV2");
	a.setOrganizationName("MCPRL");
    LabelV2 w;
    w.show();

    return a.exec();
}
