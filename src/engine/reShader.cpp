#include "reShader.h"
#include "reRadial.h"
#include "reAssetLoader.h"
#include "reTypes.h"
#include "reMaterial.h"

#include <sstream>

void printShaderInfoLog(GLuint obj)
{
	int infologLength = 0;
	int charsWritten  = 0;
	char *infoLog;

	glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

	if (infologLength > 0)
	{
		infoLog = (char *)malloc(infologLength);
		glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n",infoLog);
		if (string(infoLog).find("error") != string::npos)
		{
			__debugbreak();
		}
		free(infoLog);
	}
}

void printProgramInfoLog(GLuint obj)
{
	int infologLength = 0;
	int charsWritten  = 0;
	char *infoLog;

	glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

	if (infologLength > 0)
	{
		infoLog = (char *)malloc(infologLength);
		glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n",infoLog);
		if (string(infoLog).find("error") != string::npos)
		{
			__debugbreak();
		}
		free(infoLog);
	}
}

reShader::reShader()
{
	program = 0;
	registerVar(new reShaderVar("viewMatrix", viewMatrix));
	registerVar(new reShaderVar("projMatrix", projMatrix));
	registerVar(new reShaderVar("worldMatrix", worldMatrix));
	registerVar(new reShaderVar("diffuseTexture", diffuseTexture, reShaderVar::sampler));
	registerVar(new reShaderVar("diffuseColor",	diffuseColor));
	registerVar(new reShaderVar("cellSize",	cellSize));
	registerVar(new reShaderVar("boneMatrices",	boneMatrices));
	registerVar(new reShaderVar("lightMatrices", lightMatrices));

	registerVar(new reShaderVar("lightMatrix",	lightMatrix));
	registerVar(new reShaderVar("lightViewMatrix", lightViewMatrix));
	registerVar(new reShaderVar("lightProjMatrix", lightProjMatrix));
	registerVar(new reShaderVar("biasMatrix", biasMatrix));

	registerVar(new reShaderVar("splatTextures", splatTextures, reShaderVar::sampler, 4));
	registerVar(new reShaderVar("shadowMap", shadowMap, reShaderVar::sampler));
	registerVar(new reShaderVar("lightNearDistance", lightNearDistance));
	registerVar(new reShaderVar("lightFarDistance", lightFarDistance));
	registerVar(new reShaderVar("nearDistance", nearDistance));
	registerVar(new reShaderVar("farDistance", farDistance));
	registerVar(new reShaderVar("position",	position, reShaderVar::attribute));
	registerVar(new reShaderVar("normal", normal, reShaderVar::attribute));
	registerVar(new reShaderVar("uv)", uv, reShaderVar::attribute));
	registerVar(new reShaderVar("boneWeights", boneWeights, reShaderVar::attribute));
	registerVar(new reShaderVar("boneIds", boneIds, reShaderVar::attribute));
}													

void reShader::use()
{
	if (!program)
	{
		program = glCreateProgram();
		vs = glCreateShader(GL_VERTEX_SHADER);
		fs = glCreateShader(GL_FRAGMENT_SHADER);
		std::string vsSrc = preprocess(reRadial::shared()->assetLoader()->loadFile(vsFileName()));
		std::string fsSrc = preprocess(reRadial::shared()->assetLoader()->loadFile(fsFileName()));
		int vsize = vsSrc.size();
		int fsize = fsSrc.size();
		const char* vsrc = vsSrc.c_str();
		const char* fsrc = fsSrc.c_str();
		glShaderSource(vs, 1, &vsrc, NULL);
		glShaderSource(fs, 1, &fsrc, NULL);
		glCompileShader(vs);
		glCompileShader(fs);
		glAttachShader(program, vs);
		glAttachShader(program, fs);
		glLinkProgram(program);

		/*
		string line;
		stringstream ss(fsrc);
		int i=0;
		while (getline(ss, line))
		{
			cout << i++ << " " << line << std::endl;
		}
		*/

		printProgramInfoLog(program);
		printShaderInfoLog(vs);
		printShaderInfoLog(fs);

		collectVars();
		/*
		int sampler = 0;
		int total = -1;
		glGetProgramiv( program, GL_ACTIVE_UNIFORMS, &total ); 
		for(int i=0; i<total; ++i)  
		{
			int name_len=-1, num=-1;
			GLenum type = GL_ZERO;
			char name[100];
			glGetActiveUniform( program, GLuint(i), sizeof(name)-1,
				&name_len, &num, &type, name );
			name[name_len] = 0;
			assert(name_len < 100);
			if (name[name_len-1] == ']') name[name_len-3] = 0;
			for (int i=0; i<num; i++)
			{
				stringstream vname(name);
				if (num > 1)
				{
					vname << string(name) << "[" << i << "]";
				}
				GLuint location = glGetUniformLocation( program, vname.str().c_str() );
				uniforms[vname.str()] = location;
				if (type == GL_SAMPLER_2D)
				{
					textureUnits[vname.str()] = sampler++;
				}
				printf("%s\n", vname.str().c_str());
			}
		}
		total = -1;
		glGetProgramiv( program, GL_ACTIVE_ATTRIBUTES, &total ); 
		for(int i=0; i<total; ++i)  
		{
			int name_len=-1, num=-1;
			GLenum type = GL_ZERO;
			char name[100];
			glGetActiveAttrib( program, GLuint(i), sizeof(name)-1,
				&name_len, &num, &type, name );
			name[name_len] = 0;
			GLuint location = glGetAttribLocation( program, name );
			attributes[name] = location;
			printf("%s", name);
		}
		*/
		/*
		int len;
		glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH, &len);
		int formats;
		glGetIntegerv(GL_PROGRAM_BINARY_FORMATS, &formats);
		char *bin = new char[len];
		glProgramBinary(program)
		*/
	}
	glUseProgram(program);
	/*
	for (reVarMap::iterator it = textureUnits.begin(); it!=textureUnits.end(); it++)
	{
		for (reVarMap::iterator it2 = textureUnits.begin(); it2!=textureUnits.end(); it2++)
		{
			if (it->first !=it2->first && it->second == it2->second)
			{
				__debugbreak();
			}
		}
	}
	*/
}

std::string reShader::preprocess( std::string& src )
{
	std::stringstream ss(src), out;
	std::string line;

	for (size_t i=0; i<defines.size(); i++)
	{
		out << "#define " << defines[i] << endl;
	}
	string pragma("#pragma include");
	while (std::getline(ss, line))
	{
		if (size_t pos = line.find(pragma) != string::npos)
		{
			string include = line.substr(pos+pragma.size()+1, pos+line.size() - pragma.size()-4);
			out << reRadial::shared()->assetLoader()->loadFile("shaders/"+include) << endl;
			std::cout << include << std::endl;
		}
		else
			out << line << endl;
	}
	//std::cout << out.str() << std::endl;
	return out.str();
}

std::string reShader::fsFileName() const
{
	return _fsFileName;
}

void reShader::fsFileName( std::string val )
{
	_fsFileName = val;
}

std::string reShader::vsFileName() const
{
	return _vsFileName;
}

void reShader::vsFileName( std::string val )
{
	_vsFileName = val;
}

void reShader::unuse()
{
	glUseProgram(0);
	for (reVarMap::iterator it = textureUnits.begin(); it!=textureUnits.end(); it++)
	{
		glActiveTexture(GL_TEXTURE0+it->second);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
	}
}

void reShader::setUniform( int name, float v, int index )
{
	glUniform1f(location(name, index), v);
}

void reShader::setUniform( int name, int v, int index  )
{
	glUniform1i(location(name, index), v);
}

void reShader::setUniform( int name, int count, int* v, int index  )
{
	glUniform1iv(location(name, index), count, v);
}

void reShader::setUniform( int name, reVec2& v, int index  )
{
	glUniform2f(location(name, index), v.x, v.y);
}

void reShader::setUniform( int name, reVec4& v, int index  )
{
	glUniform4f(location(name, index), v.x, v.y, v.z, v.w);
}

void reShader::setUniformMatrix( int name, int count, GLfloat* matrices, int index )
{	
	glUniformMatrix4fv(location(name, index), count, false, matrices);
}

void reShader::setTexture( int name, reTexture* texture, int index )
{
	setUniform(name, varTable[name]->index, index);
	texture->bind(varTable[name]->index);
}

void reShader::setTexture( int name, GLuint texture, int index )
{
	setUniform(name, varTable[name]->index, index);
	glActiveTexture(GL_TEXTURE0+varTable[name]->index);	
	glBindTexture(GL_TEXTURE_2D, texture);
	glEnable(GL_TEXTURE_2D);
}

void reShader::registerVar( reShaderVar* uniform )
{
	if (varTable.size() <= uniform->id)
	{
		varTable.resize(uniform->id+1);
	}
	varTable[uniform->id] = uniform;
}

void reShader::collectVars()
{
	int samplers = 0, attrs = 0;
	for (int i=0; i<varTable.size(); i++)
	{
		switch (varTable[i]->kind)
		{
		case reShaderVar::sampler:			
		case reShaderVar::uniform:
			varTable[i]->location = glGetUniformLocation(program, varTable[i]->name.c_str());
			if (varTable[i]->location != -1 && varTable[i]->kind == reShaderVar::sampler)
			{
				varTable[i]->index = samplers;
				samplers += varTable[i]->size;
			}
			break;
		case reShaderVar::attribute:
			varTable[i]->location = glGetAttribLocation(program, varTable[i]->name.c_str());
			varTable[i]->index = attrs++;
			break;
		}
	}
}

int reShader::location( int name, int index )
{
	return name<varTable.size() ? varTable[name]->location + index : -1;
}
