#include "reVertexBuffer.h"
#include "reUtilities.h"
#include "reMesh.h"

reVertexBuffer::reVertexBuffer()
{
	vbo = 0;
}

reVertexBuffer::~reVertexBuffer()
{
	if(vbo)
		glDeleteBuffers(1, &vbo);
}

void reVertexBuffer::load( reVertex* vertices, int count )
{
	if (vbo)
	{
		glDeleteBuffers(1, &vbo);
	}
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, count * sizeof(reVertex), vertices, GL_STATIC_DRAW);
}

void reVertexBuffer::bindPointers(reShader* shader)
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexPointer(3, GL_FLOAT, sizeof(reVertex), (GLvoid*)offsetof(reVertex, pos)); // pos
	glTexCoordPointer(2, GL_FLOAT, sizeof(reVertex), (GLvoid*)offsetof(reVertex, uv)); // uv
	glNormalPointer(GL_FLOAT, sizeof(reVertex), (GLvoid*)offsetof(reVertex, normal)); // normal
	glEnableVertexAttribArray(shader->location(reShader::boneIds));	
	glVertexAttribPointer(shader->location(reShader::boneIds), 4, GL_FLOAT, false, sizeof(reVertex), (GLvoid*)offsetof(reVertex, bones));
	glEnableVertexAttribArray(shader->location(reShader::boneWeights));
	glVertexAttribPointer(shader->location(reShader::boneWeights), 4, GL_FLOAT, false, sizeof(reVertex), (GLvoid*)offsetof(reVertex, weights));
}

void reVertexBuffer::unbind()
{
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
}

