#include "reTerrainPanel.h"
#include "reTerrainModifier.h"
#include "reMapBrush.h"
#include "reEditor.h"
#include "reTerrainMaterial.h"
#include "reTerrainNode.h"

#include <QDirIterator>
//#include <QFileInfo>
#include <QDebug>
#include "reTextureDialog.h"

reTerrainPanel::reTerrainPanel( QWidget* parent /*= 0*/ ): 
	QWidget(parent)
{
	setupUi(this);
	/*
	tableLayers->setIconSize(QSize(64, 64));	
	QDirIterator it(QString(reEditor::instance()->project()->root().c_str()) + "/materials/terrain/", QDirIterator::Subdirectories);
	while (it.hasNext()) 
	{
		QFileInfo info(it.next());
		if (!info.isDir())
		{
			cmbMaterial->addItem(info.baseName(), info.absoluteFilePath());
		}
	}
	*/
}

reTerrainModifier* reTerrainPanel::modifier() const
{
	return _modifier;
}

void reTerrainPanel::modifier( reTerrainModifier* val )
{
	_modifier = val;

}

void reTerrainPanel::sizeSliderChanged()
{
	if (modifier()->brush())
	{
		modifier()->brush()->size(sliderSize->value() / 100.0f);
	}	
}

void reTerrainPanel::speedSliderChanged()
{
	if (modifier()->brush())
	{
		modifier()->brush()->speed(sliderSpeed->value() / 100.0f);
	}
}

void reTerrainPanel::intensitySliderChanged()
{
	if (modifier()->brush())
	{
		modifier()->brush()->speed(sliderIntensity->value() / 100.0f);
	}	
}

void reTerrainPanel::terrainButtonClicked(QAbstractButton * button)
{
	if (button == btnLower)
	{
		reHeightBrush* brush = new reHeightBrush;
		brush->direction = reHeightBrush::down;
		modifier()->brush(brush);
	}
	if (button == btnUpper)
	{
		reHeightBrush* brush = new reHeightBrush;
		brush->direction = reHeightBrush::up;
		modifier()->brush(brush);
	}
	if (button == btnUniform)
	{
		reUniformBrush* brush = new reUniformBrush;
		modifier()->brush(brush);
	}
	if (button == btnSmooth)
	{
		reSmoothBrush* brush = new reSmoothBrush;
		modifier()->brush(brush);
	}
	updateUI();
}

void reTerrainPanel::updateUI()
{
	if (modifier()->brush())
	{
		sliderSize->setValue(modifier()->brush()->size() * 100);
		sliderSpeed->setValue(modifier()->brush()->speed() * 100);
		sliderIntensity->setValue(modifier()->brush()->intensity() * 100);
	}
}

void reTerrainPanel::cmbMaterialChanged( int idx )
{
	/*
	QString path = cmbMaterial->itemData(idx, Qt::UserRole).toString();
	QDirIterator it(path);

	int i=0;
	tableLayers->setRowCount(0);
	while (it.hasNext())
	{		
		QFileInfo info(it.next());
		if (!info.isFile()) continue;

		//tableLayers->setRowCount(i);
		tableLayers->insertRow(i);

		QTableWidgetItem* item = new QTableWidgetItem();
		item->setIcon(QIcon(info.absoluteFilePath()));
		item->setTextAlignment(Qt::AlignCenter);
		item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
		tableLayers->setItem(i, 0, item);
		item = new QTableWidgetItem(info.baseName());
		tableLayers->setItem(i, 1, item);
		item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
		tableLayers->setRowHeight(i, 70);
		item->setData(Qt::UserRole, info.absoluteFilePath());
		i++;
	}
	*/
}
/*
void reTerrainPanel::targetLayerClicked( QTableWidgetItem* )
{
	reSplatBrush* brush = new reSplatBrush;
	modifier()->brush(brush);
}

void reTerrainPanel::targetLayerDblClicked( QTableWidgetItem* )
{
	reTextureDialog dialog(this);
	dialog.exec();
}*/