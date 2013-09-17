#ifndef reTransform_h__
#define reTransform_h__

#include "reTypes.h"
#include "reVar.h"

class reTransform
{	
public:
	reMat4 matrix;
	reMat4 inverseMatrix();

	reTransform();
	reTransform(const reMat4& matrix);
	reTransform(reVar& ret);
	operator reVar() const;
	reVar asVar();
	void fromVar(reVar var);

	void rotation(reVec3 pos);;
	reVec3 rotation () const;

	void position(reVec3 pos);
	reVec3 position() const;

	void scale( reVec3 scale );
	reVec3 scale() const;

	void decompose(reVec3& translation, reMat4& rotation, reVec3& scale) const;
	void decompose(reVec3& translation, reVec3& rotation, reVec3& scale) const;
	void compose(reVec3& translation, reVec3& rotation, reVec3& scale);
	
};

#endif // reTransform_h__