#ifndef reText_h__
#define reText_h__

#include "reRenderable.h"
#include <vector>
#include <map>

class reMaterial;

struct reFontChar
{
	float u0, v0, u1, v1;
	float xoffset, yoffset;
	float xadvance;
	float width, height;
};

typedef std::map<char, reFontChar*> reFontCharMap;

class reFont
{
public:
	reMaterial* material;
	float lineHeight;
	reFont();
	virtual ~reFont();
	reFontCharMap chars;
	void loadFromFile(const string& fileName);
};

class reText: public reRenderable
{
public:
	string _text;
	std::string text() const;
	void text(std::string val);
	reFont* _font;
	reFont* font() const;
	void font(reFont* val);
	void font(string val);
	reText();
	virtual ~reText();
	void setFont(string name);
	virtual void draw(int state = 0, int id = 0);
};

#endif // reText_h__