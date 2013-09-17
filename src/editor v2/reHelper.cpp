#include "std.h"
#include "reHelper.h"

#include "core.h"
#include "reContextWidget.h"
#include "reContextSelectionModel.h"

reSelectionHelper::reSelectionHelper( reContextSelectionModel* model ): 
reHelper()
{
	this->model = model;
}

void drawQuad(reVec3 a, reVec3 b) 
{
	glBegin(GL_LINE_STRIP);
	glVertex3f(a[0], a[1], a[2]);
	glVertex3f(a[0], a[1], b[2]);
	glVertex3f(a[0], b[1], b[2]);
	glVertex3f(b[0], b[1], b[2]);
	glVertex3f(b[0], b[1], a[2]);
	glVertex3f(b[0], a[1], a[2]);
	glVertex3f(a[0], a[1], a[2]);
	glEnd();
}

void drawBBox(reBBox bBox)
{
	reVec3& a = bBox.pMin;
	reVec3& b = bBox.pMax;

	glDisable(GL_CULL_FACE);
	
	//glScalef(1.3f, 1.3f, 1.3f);
	glColor4f(.5, .5, .5, 1);
	glLineWidth(1);

	drawQuad(reVec3(a[0], a[1], a[2]), reVec3(a[0], b[1], b[2]));
	drawQuad(reVec3(a[0], a[1], a[2]), reVec3(b[0], a[1], b[2]));
	drawQuad(reVec3(a[0], a[1], a[2]), reVec3(b[0], a[1], a[2]));
	drawQuad(reVec3(b[0], b[1], b[2]), reVec3(b[0], a[1], a[2]));
	drawQuad(reVec3(b[0], b[1], b[2]), reVec3(a[0], b[1], a[2]));
	drawQuad(reVec3(b[0], b[1], b[2]), reVec3(a[0], b[1], b[2]));

	glLineWidth(1);
	glEnable(GL_CULL_FACE);
}

void reSelectionHelper::render( int state /*= 0*/ )
{	
	foreach (QModelIndex idx, model->selectedIndexes())
	{
		reNode* node = dynamic_cast<reNode*>((reObject*)idx.internalPointer());
		if (!node) continue;
		node->render(reRS_WIREFRAME);
		glPushMatrix();
		glMultMatrixf(glm::value_ptr(node->worldTransform().matrix));
		drawBBox(node->bBox());
		glPopMatrix();
	}
}
