#include "std.h"
#include "reMaker.h"

#include "reObject.h"
#include "reNode.h"
#include "reMesh.h"
#include "reBone.h"
#include "rePTerrain.h"
#include "reRadial.h"
#include "reAssetLoader.h"
#include "reAnimationSet.h"
#include "rePhysics.h"
#include "rePlayer.h"
#include "reCamera.h"
#include "reLight.h"
#include "reEnemy.h"
#include "reScatter.h"
#include "reRunner.h"
#include "reBuilding.h"

reMaker* instance = 0;

reMaker::reMaker()
{
	glWidget = 0;
	registerType(new reType<reObject>("reObject", QStringList(), "obj", QIcon(":/cog.png")));
	registerType(new reType<reCollection>("reCollection", QStringList("reNode"), "col", QIcon(":/folder.png")));
	registerType(new reType<reScatter>("reScatter", QStringList("reNode"), "sct", QIcon(":/scatter.png")));
	registerType(new reType<reBuilding>("reBuilding", QStringList("reNode"), "building", QIcon(":/scatter.png")));
	registerType(new reType<rePhysics>("rePhysics", QStringList(), "phy"));
	registerType(new reType<reBody>("reBody", QStringList(), "body"));
	//registerType(new reType<reRefNode>("reRefNode", QStringList(), "ref", QIcon(":/ref.png")));
	registerType(new reType<reNode>("reNode", QString("reAnimator,reBody,rePhysics,reSequence,reCollection").split(","), "node", QIcon(":/cog.png")));
	registerType(new reType<reBone>("reBone", QString("reAnimator").split(","), "bone", QIcon(":/cog.png")));
	registerType(new reType<rePTerrain>("rePTerrain", QStringList(), "pter", QIcon(":/cog.png")));
	registerType(new reType<reNodeCollection>("reNodeCollection", QString("reNode,rePTerrain,rePlayer,reScatter,reBuilding").split(","), "nc", QIcon(":/folder.png")));
	registerType(new reType<reRenderableCollection>("reRenderableCollection", QString("reRenderable,reMesh").split(","), "rc", QIcon(":/folder.png")));
	registerType(new reType<reMesh>("reMesh", QStringList(), "mesh", QIcon(":/mesh.png")));
	registerType(new reType<reAnimator>("reAnimator", QString("reSequence").split(","), "anim", QIcon(":/film.png")));
	registerType(new reType<reSequence>("reSequence", QStringList(), "seq", QIcon(":/clapper.png")));

	registerType(new reType<rePlayer>("rePlayer", QString("rePointCamera,reLight").split(","), "player", QIcon(":/cog.png")));
	registerType(new reType<reEnemy>("reEnemy", QString("rePointCamera,reLight").split(","), "enemy", QIcon(":/cog.png")));
	registerType(new reType<reLight>("reLight", QStringList(), "light", QIcon(":/cog.png")));
	registerType(new reType<rePointCamera>("rePointCamera", QStringList(), "cam", QIcon(":/cog.png")));
	registerType(new reType<reRunner>("reRunner", QStringList(), "run", QIcon(":/cog.png")));
}

reMaker* reMaker::shared()
{
	return instance ? instance : (instance = new reMaker);
}

QString reMaker::root() const
{
	return _root;
}

void reMaker::root( QString val )
{	
	if (_root != val)
	{
		_root = val;
		reRadial::shared()->assetLoader()->addPath(_root.toStdString());
		emit projectSet();
	}
}

void reMaker::shutdown()
{
	emit shuttingDown();
}

void reMaker::registerType( reTypeEntry* entry )
{
	types.push_back(entry);
}

const reTypeEntry* reMaker::getTypeInfo( QString className )
{
	int i=0;
	foreach (reTypeEntry* type, types)
	{
		if (type->className == className) 
			return types.at(i);
		i++;
	}
	return types[0];
}

const reTypeEntry* reMaker::getTypeInfoByExt( QString ext )
{
	int i=0;
	foreach (reTypeEntry* type, types)
	{
		if (type->shortName == ext) 
			return types.at(i);
		i++;
	}
	return 0;
}

int getTime()
{
	SYSTEMTIME time;
	GetSystemTime(&time);
	return time.wMilliseconds + time.wSecond * 1000;
}


void reMaker::initGL(QWidget* cnt)
{
	/*
	PIXELFORMATDESCRIPTOR pfd;
	HWND hwnd; HDC hdc; int pixelFormat;
	memset(&pfd,0,sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;
	hwnd = CreateWindowEx(WS_EX_TOOLWINDOW,"STATIC","glctx",
		WS_POPUP|WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 
		0,0,640,480,0,0,GetModuleHandle(NULL),0); 
	ShowWindow(hwnd,SW_HIDE);
	
	hdc = GetDC(hwnd);
	pixelFormat = ChoosePixelFormat(hdc,&pfd);
	SetPixelFormat(hdc,pixelFormat,&pfd);
	wglCreateContext(hdc);
	//wglMakeCurrent(hdc,wglCreateContext(hdc));
	*/
	qDebug() << "started initializing GL";
	int time = getTime();

	QGLFormat glf = QGLFormat::defaultFormat();
	// glf.setVersion(3, 2);
	//qDebug() << (glf.openGLVersionFlags() & QGLFormat::OpenGL_Version_4_0);
	/*
	glf.setSampleBuffers(true);
	glf.setSamples(4);
	glf.setAlpha(true);
	glf.setRgba(true);
	glf.setDoubleBuffer(true);
	glf.setSwapInterval(10);
	glf.setDirectRendering(true);
	glf.setSampleBuffers(true);
	QGLFormat::setDefaultFormat(glf);
	*/
	
	glWidget = new QGLWidget(glf);
	glWidget->makeCurrent();
	GLenum err = glewInit();
	
	//ctx = new QGLContext(glf);
	//qDebug() << ctx->create();
	//ctx->makeCurrent();	
	
	if (GLEW_OK != err)
	{
		qDebug() << "error ! : " << (char*)glewGetErrorString(err);
	}
	qDebug() << "Status: Using GLEW %s\n" << (char*)glewGetString(GLEW_VERSION);

	qDebug() << "initialized GL";
	qDebug() << endl << "init took:" << getTime() - time << " miliseconds" << endl; time = getTime();

	
	
	/*
	QGLFormat glf = QGLFormat::defaultFormat();

	glf.setSampleBuffers(true);
	glf.setSamples(4);
	glf.setDoubleBuffer(true);
	glf.setDirectRendering(true);
	glf.setSampleBuffers(true);

	QGLFormat::setDefaultFormat(glf);
	*/
		


	//QGLContext* ctx = new QGLContext(glf);
	//ctx->makeCurrent();


}