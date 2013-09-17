#ifndef reTextureDialog_h__
#define reTextureDialog_h__

#include "ui_texturedialog.h"

#include <QDialog>

class reTextureDialog: public QDialog, public Ui::TextureDialog
{
	Q_OBJECT
public:
	reTextureDialog(QWidget* parent=0);
	virtual void reject();
	virtual void accept();
public slots:
	void textureSelect();
};

#endif // reTextureDialog_h__