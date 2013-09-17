#include "reMaterial.h"
#include "reRadial.h"
#include "reAssetLoader.h"

#include <iostream>
#include "reShader.h"
#include "core.h"

reMaterial::reMaterial():
	diffuseColor(1, 1, 1, 1), id(0)
{
	diffuseTexture = NULL;
	isTransparent = false;
}

reMaterial::~reMaterial()
{
	reRadial::shared()->assetLoader()->releaseShared(diffuseTexture);
}

void reMaterial::bind()
{
	if (diffuseTexture)
	{
		glEnable(GL_TEXTURE_2D);
		diffuseTexture->bind(0);		
		glColor4f(1, 1, 1, 1);
	} 
	else
	{
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
		glColor4f(diffuseColor.r, diffuseColor.g, diffuseColor.b, diffuseColor.a);
	}	
}

void reMaterial::unbind()
{
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void reMaterial::save( std::ostream& os )
{
	os.write((char*)&id, sizeof(id));
	os.write((char*)&isTransparent, sizeof(isTransparent));
	os.write((char*)&diffuseColor, sizeof(diffuseColor));
	bool diffuseTextureExists = diffuseTexture != 0;
	os.write((char*)&diffuseTextureExists, sizeof(diffuseTextureExists));
	if (diffuseTextureExists)
	{
		diffuseTexture->save(os);
	}
}

void reMaterial::load( std::istream& is )
{
	is.read((char*)&id, sizeof(id));
	is.read((char*)&isTransparent, sizeof(isTransparent));
	is.read((char*)&diffuseColor, sizeof(diffuseColor));
	bool diffuseTextureExists;
	is.read((char*)&diffuseTextureExists, sizeof(diffuseTextureExists));
	if (diffuseTextureExists)
	{
		diffuseTexture = new reTexture;
		string _fileName;
		unsigned int size;
		is.read((char*)&size, sizeof(size));
		_fileName.resize(size);
		is.read(&_fileName[0], size);
		diffuseTexture = reRadial::shared()->assetLoader()->loadShared<reTexture>(_fileName);
	}
}

void reMaterial::updateShader( reShader* shader )
{
	//bind();
	if ((reEFFECT_DIFFUSE_COLOR | reEFFECT_DIFFUSE_TEXTURE) & shader->effects)
	{
		shader->setUniform(reShader::diffuseColor, diffuseColor);
		if(diffuseTexture)
		{
			shader->setTexture(reShader::diffuseTexture, diffuseTexture);
		}
	}
}

int reMaterial::getEffect()
{
	return diffuseTexture ? reEFFECT_DIFFUSE_TEXTURE  | reEFFECT_DIFFUSE_COLOR: reEFFECT_DIFFUSE_COLOR;
}

bool reMaterial::operator==( const reMaterial& other ) const
{

	return (this->diffuseTexture ? this->diffuseTexture->fileName() : string("")) == (other.diffuseTexture ? other.diffuseTexture->fileName(): string(""))
	&&
	diffuseColor == other.diffuseColor;
}

//////////////////////////////////////////////////////////////////////////

reTexture::reTexture()
{
	loaded = false;
}

reTexture::~reTexture()
{
	glDeleteTextures(1, &glName);
}

std::string reTexture::fileName() const
{
	return _fileName;
}

void reTexture::fileName( std::string val )
{
	_fileName = val;
}

void reTexture::load() 
{
	reImage image;
	reRadial::shared()->assetLoader()->loadImage(fileName(), &image);
	glGenTextures(1, &glName);
	glBindTexture(GL_TEXTURE_2D, glName);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA,
		image.width, image.height,
		0, GL_RGBA, GL_UNSIGNED_BYTE, image.data );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ); 
	loaded = true;
}

void reTexture::bind( int unit )
{
	if (!loaded)
	{
		load();
	}
	glActiveTexture(GL_TEXTURE0 + unit);	
	glBindTexture(GL_TEXTURE_2D, glName);
	glEnable(GL_TEXTURE_2D);
}

void reTexture::save( std::ostream& os )
{
	unsigned int size = fileName().size();
	os.write((char*)&size, sizeof(size));	
	os.write(reRadial::shared()->assetLoader()->relativePath(fileName()).c_str(), size);
}

void reTexture::load( std::istream& is )
{
	unsigned int size;
	is.read((char*)&size, sizeof(size));
	_fileName.resize(size);
	is.read(&_fileName[0], size);
}

void reTexture::loadFromFile( std::string fileName_ )
{
	fileName(fileName_);
}

//////////////////////////////////////////////////////////////////////////

void reMaterialSet::addMetarial( reMaterial* material )
{
	materials.push_back(material);
}

reMaterial* reMaterialSet::materialById( int id )
{
	for (reMaterialVector::iterator it=materials.begin(); it != materials.end(); it++)
	{
		if ((*it)->id == id) return (*it);
	}
	if (id==-1)
	{
		addMetarial(new reMaterial);
	}
	return 0;
	//return materialById(-1);
}


void reMaterialSet::save( std::ostream& os )
{
	unsigned int count = materials.size();
	os.write((char*)&count, sizeof(count));
	for (reMaterialVector::iterator it=materials.begin(); it!=materials.end(); it++)
	{
		(*it)->save(os);
	}
}

void reMaterialSet::load( std::istream& is )
{
	unsigned int count;
	is.read((char*)&count, sizeof(count));
	for (unsigned int i = 0; i<count; i++)
	{
		reMaterial* mat = new reMaterial;
		mat->load(is);
		materials.push_back(mat);
	}
}

reImage::~reImage()
{
	free(data);
}