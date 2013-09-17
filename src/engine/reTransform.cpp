#include "reTransform.h"
#include "core.h"
#include <cmath>

reMat4 reTransform::inverseMatrix()
{
	return glm::inverse(matrix);
}

reTransform::reTransform( const reMat4& mat ):
	matrix(mat)
{

}

reTransform::reTransform()
{

}

reTransform::reTransform( reVar& ret )
{
	float x, y, z, rx, ry, rz, sx, sy, sz;
	x = ret["position"]["x"].asFloat();
	y = ret["position"]["y"].asFloat();
	z = ret["position"]["z"].asFloat();
	rx = ret["rotation"]["x"].asFloat();
	ry = ret["rotation"]["y"].asFloat();
	rz = ret["rotation"]["z"].asFloat();
	sx = ret["scale"]["x"].asFloat();
	sy = ret["scale"]["y"].asFloat();
	sz = ret["scale"]["z"].asFloat();
	compose(reVec3(x, y, z), reVec3(rx, ry, rz), reVec3(sx, sy, sz));
}

void reTransform::position( reVec3 pos )
{
	reVec3 t, s, r;
	decompose(t, r, s);
	compose(pos, r, s);
}

reVec3 reTransform::position() const
{
	reVec3 t, s; reMat4 r;
	decompose(t, r, s);
	return t;
}

reVec3 reTransform::rotation() const
{
	reVec3 t, r, s;
	decompose(t, r, s);
	return r;
}

void reTransform::rotation( reVec3 euler )
{
	reVec3 t, r, s;
	decompose(t, r, s);
	compose(t, euler, s);
}

void reTransform::scale( reVec3 scale )
{
	reVec3 t, r, s;
	decompose(t, r, s);
	compose(t, r, scale);
}

reVec3 reTransform::scale() const
{
	reVec3 t, r, s;
	decompose(t, r, s);
	return s;
}

reTransform::operator reVar() const
{
	reVec3 p, s, r;
	decompose(p, r, s);

	reVar ret;
	ret["position"]["x"] = p.x;
	ret["position"]["y"] = p.y;
	ret["position"]["z"] = p.z;
	ret["rotation"]["x"] = r.x;
	ret["rotation"]["y"] = r.y;
	ret["rotation"]["z"] = r.z;
	ret["scale"]["x"] = s.x;
	ret["scale"]["y"] = s.y;
	ret["scale"]["z"] = s.z;
	return ret;
}

reVar reTransform::asVar()
{
	reVar transformValues;
	reTransform transform;

	for (unsigned int i=0; i<16; i++)
	{
		transformValues[i] = matrix[i/4][i%4];
	}
	return transformValues;
}

void reTransform::fromVar( reVar transformValues )
{
	for (unsigned int i=0; i<transformValues.size(); i++)
	{
		matrix[i/4][i%4] = transformValues[i].asFloat();
	}
}

void reTransform::decompose( reVec3& translation, reMat4& rotation, reVec3& scale ) const
{
	reMat4 mCopy(matrix);
	translation = reVec3(mCopy[3][0], mCopy[3][1], mCopy[3][2]);
	for ( size_t i = 0; i < 3; i++) {
		mCopy[i][3] = mCopy[3][i] = 0.0;
	}
	mCopy[3][3] = 1.0;

	// Extract the rotation component - this is done using polar decompostion, where
	// we successively average the matrix with its inverse transpose until there is
	// no/a very small difference between successive averages
	float norm;
	int count = 0;
	rotation = mCopy;
	do {
		reMat4 nextRotation;
		reMat4 currInvTranspose = 
			glm::inverse(glm::transpose(rotation));

		// Go through every component in the matrices and find the next matrix
		for ( size_t i = 0; i < 4; i++) {
			for ( size_t j = 0; j < 4; j++) {
				nextRotation[i][j] = 0.5f * (rotation[i][j] + currInvTranspose[i][j]);
			}
		}

		norm = 0.0;
		for ( size_t i = 0; i < 3; i++) {
			float n = abs(rotation[i][0] - nextRotation[i][0]) +
				abs(rotation[i][1] - nextRotation[i][1]) +
				abs(rotation[i][2] - nextRotation[i][2]);
			norm = max(norm, n);
		}
		rotation = nextRotation;
	} while (count++ < 100 && norm > 0.00000001);

	// The scale is simply the removal of the rotation from the non-translated matrix
	reMat4 scaleMatrix = glm::inverse(rotation) * mCopy;
	scale = reVec3(scaleMatrix[0][0],
		scaleMatrix[1][1],
		scaleMatrix[2][2]);

	// Calculate the normalized rotation matrix and take its determinant to determine whether
	// it had a negative scale or not...
	reVec3 row1(mCopy[0][0], mCopy[0][1], mCopy[0][2]);
	reVec3 row2(mCopy[1][0], mCopy[1][1], mCopy[1][2]);
	reVec3 row3(mCopy[2][0], mCopy[2][1], mCopy[2][2]);
	row1 = glm::normalize(row1);
	row2 = glm::normalize(row2);
	row3 = glm::normalize(row3);
	reMat3 nRotation(row1, row2, row3);

	// Special consideration: if there's a single negative scale 
	// (all other combinations of negative scales will
	// be part of the rotation matrix), the determinant of the 
	// normalized rotation matrix will be < 0. 
	// If this is the case we apply an arbitrary negative to one 
	// of the component of the scale.
	float determinant = glm::determinant(nRotation);
	if (determinant < 0.0) {
		scale.x *= -1;
	}
}

void reTransform::decompose( reVec3& translation, reVec3& rotation, reVec3& scale ) const
{
	reMat4 rot;
	decompose(translation, rot, scale);
	rotation = glm::eulerAngles(glm::quat(rot));	
}

void reTransform::compose( reVec3& translation, reVec3& rotation, reVec3& scale )
{
	reMat4 r = 
		glm::rotate(reMat4(), rotation[2], reVec3(0, 0, 1)) *		
		glm::rotate(reMat4(), rotation[1], reVec3(0, 1, 0)) *
		glm::rotate(reMat4(), rotation[0], reVec3(1, 0, 0)) *
									
		reMat4();
	reMat4 t(glm::translate(reMat4(), translation));
	reMat4 s(glm::scale(reMat4(), scale));
	matrix = t * r * s;
}
