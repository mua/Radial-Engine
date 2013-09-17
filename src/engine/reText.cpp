#include "reText.h"
#include "xml/pugixml.hpp"

#include "core.h"
#include "reUtilities.h"
#include "reMaterial.h"
#include "reRadial.h"
#include "reAssetLoader.h"

#include <iostream>

using namespace pugi;
using namespace std;

reText::reText():
	_font(0), _text("")
{
	
}

reText::~reText()
{
	delete font();
}

std::string reText::text() const
{
	return _text;
}

void reText::text( std::string val )
{
	_text = val;
}

reFont* reText::font() const
{
	return _font;
}

void reText::font( reFont* val )
{
	_font = val;
}

void reText::font( string val )
{
	reFont* f = new reFont();
	f->loadFromFile(val);
	font(f);
}

void reText::draw( int state /*= 0*/, int id /*= 0*/ )
{
	if (!state)
	{
		font()->material->bind();
		glColor4f(1,1,1,1);
	}
	float cx = 0, cy = 0;
	glBegin(GL_QUADS);
	for (unsigned int i=0; i<text().size(); i++)
	{
		reFontChar* chr = font()->chars[text()[i]];		
		float x = cx + chr->xoffset;
		float y = -chr->height - chr->yoffset + font()->lineHeight;
		glTexCoord2f(chr->u0, chr->v0); glVertex3f(x, y, 0);
		glTexCoord2f(chr->u1, chr->v0); glVertex3f(x+chr->width, y, 0);
		glTexCoord2f(chr->u1, chr->v1); glVertex3f(x+chr->width, y+chr->height, 0);
		glTexCoord2f(chr->u0, chr->v1); glVertex3f(x, y+chr->height, 0);		
		cx += chr->xadvance;
	}
	glEnd();
}

//////////////////////////////////////////////////////////////////////////

reFont::reFont()
{

}

void reFont::loadFromFile( const string& fileName )
{
	string filePath = reRadial::shared()->assetLoader()->filePath(fileName);
	cout << filePath << endl;
	xml_document doc;
	xml_parse_result result = doc.load_file(filePath.c_str());
	xml_node node = doc.child("font").child("chars");
	float w = doc.child("font").child("common").attribute("scaleW").as_float();
	float h = doc.child("font").child("common").attribute("scaleH").as_float();
	lineHeight = doc.child("font").child("common").attribute("lineHeight").as_float();
	for (pugi::xml_node fchar = node.child("char"); fchar; fchar = fchar.next_sibling("char"))
	{
		reFontChar* fc = new reFontChar;
		fc->width = fchar.attribute("width").as_float();
		fc->height = fchar.attribute("height").as_float();
		fc->u0 = fchar.attribute("x").as_float() / w;
		fc->v0 = 1 - (fchar.attribute("y").as_float() + fc->height) / h;
		fc->u1 = fc->u0 + (fc->width / w);
		fc->v1 = fc->v0 + (fc->height / h);
		fc->xadvance = fchar.attribute("xadvance").as_float();
		fc->xoffset = fchar.attribute("xoffset").as_float();
		fc->yoffset = fchar.attribute("yoffset").as_float();
		chars[fchar.attribute("id").as_int()] = fc;
	}
	material = new reMaterial();
	material->diffuseTexture = new reTexture();
	material->diffuseTexture->fileName( dirPath(filePath) + "/" + doc.child("font").child("pages").child("page").attribute("file").as_string());
	cout << material->diffuseTexture->fileName() << endl;
}

reFont::~reFont()
{
	for (reFontCharMap::iterator it = chars.begin(); it != chars.end(); it++)
	{
		delete it->second;
	}
}
