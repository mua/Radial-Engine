#include "reTextureDialog.h"
#include <QDebug>
#include <QFileDialog>
#include "reEditor.h"

reTextureDialog::reTextureDialog( QWidget* parent/*=0*/ ):
	QDialog(parent)
{
	setupUi(this);
}

void reTextureDialog::reject()
{
	QDialog::reject();
	qDebug() << "rejected";
}

void reTextureDialog::accept()
{
	QDialog::accept();
	qDebug() << "accepted";
}

void reTextureDialog::textureSelect()
{
	QString fileName = QFileDialog::getOpenFileName(this, "Load Texture", reEditor::instance()->project()->root().c_str(), "Image Files (*.png *.jpg *.bmp)");
	if (!fileName.isEmpty())
		qDebug() << "texture selected" + fileName;
}