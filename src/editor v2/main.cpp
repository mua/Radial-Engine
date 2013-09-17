#include <gl/glew.h>

#include "std.h"

#include "reMaker.h"
#include "reMainWindow.h"
#include "reRadial.h"
#include "reMakerAssetLoader.h"

#include "btBulletDynamicsCommon.h"

#include <QDir>
#include <QFileInfo>
#include "reTypes.h"
#include "reTransform.h"

int main(int argc, char *argv[])
{

#if defined(_MSC_VER) && defined(_DEBUG)
	//_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_EVERY_16_DF);
#endif

	QGLFormat glf = QGLFormat::defaultFormat();
	glf.setSampleBuffers(true);
	//glf.setSamples(16);
	glf.setDoubleBuffer(true);
	glf.setDirectRendering(true);
	//glf.setSampleBuffers(true);
	QGLFormat::setDefaultFormat(glf);

	reMakerAssetLoader* loader = new reMakerAssetLoader;
	reRadial::shared()->assetLoader(loader);
	loader->addPath(QFileInfo(argv[0]).absoluteDir().absoluteFilePath("resource").toStdString());

	QApplication app(argc, argv);
	app.addLibraryPath(QFileInfo(argv[0]).absoluteDir().absoluteFilePath("plugins"));
	QCoreApplication::setOrganizationDomain("Com");
	QCoreApplication::setOrganizationName("Crimson");
	QCoreApplication::setApplicationName("Radial");
	QCoreApplication::setApplicationVersion("1.0.0");
	reMainWindow* mainWindow = new reMainWindow();
	reMaker::shared()->mainWindow = mainWindow;
	mainWindow->show();
	return app.exec();
}