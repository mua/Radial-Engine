#ifndef reTerrainPanel_h__
#define reTerrainPanel_h__

#include "ui_terrain.h"
#include "reCollectionModel.h"

class reTerrainModifier;

class reMaterialModel: public reContextModel
{

};

class reTerrainPanel: public QWidget, public Ui::Terrain
{
	Q_OBJECT
private:
	reTerrainModifier* _modifier;
public:
	reTerrainPanel(QWidget* parent = 0);
	reTerrainModifier* modifier() const;
	void modifier(reTerrainModifier* val);
	void updateUI();
public slots:
	void sizeSliderChanged();
	void speedSliderChanged();
	void intensitySliderChanged();
	void terrainButtonClicked(QAbstractButton * button);
	void cmbMaterialChanged(int);
	//void targetLayerClicked(QTableWidgetItem*);
	//void targetLayerDblClicked(QTableWidgetItem*);
};

#endif // reTerrainPanel_h__
