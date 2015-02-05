#include <QApplication>
#include "mewindow.h"
#include "global.h"
//==================================================================================

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
//	app.setWindowIcon(QIcon(":/22/pixmaps/22/text-speak.png"));
    Global::initPath();
    Global::initTranslator(Global::menuEditorName());

    MeWindow win;
    win.show();

    return app.exec();
}
