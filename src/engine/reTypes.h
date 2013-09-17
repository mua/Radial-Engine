#ifndef reTypes_h__
#define reTypes_h__

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include <vector>
#include <string>

typedef glm::vec2 reVec2;
typedef glm::vec3 reVec3;
typedef glm::vec4 reVec4;
typedef glm::vec4 reColor4;
typedef glm::mat4 reMat4;
typedef glm::mat3 reMat3;
typedef glm::quat reQuat;

typedef std::vector<int> reIntVector;
typedef std::vector<std::string> reStringList;

struct reBBox
{
	bool isEmpty;
	reVec3 pMax, pMin;
	reBBox();
	reBBox(reVec3 max, reVec3 min);
	void addPoint( const reVec3& pos );
	void addBox(const reBBox& box);
	void addBox(const reBBox& box, const reMat4& transform);
	void getCorners(reVec3 points[]) const;
	reVec3 center();
};

struct reRay
{
	reVec3 p, n;
	reRay() 
	{

	};
	reRay(const reVec3& p, const reVec3& n)
	{
		this->p = p; 
		this->n = n;
	};
};

struct reFrustum
{
	reRay left, top, right, bottom, start, end;
	bool limitless;
	reFrustum();
	void normalize();
	bool pointInside(const reVec3& v);
	void fromMatrix(reMat4& mat);
	int classifyBBox(reBBox& bbox);
	void transform(const reMat4& mat);
};

/*
struct reVec2
{
	float x, y;
	reVec2();
	reVec2(const reVec2& vec);
	reVec2(float _x, float _y);
};

struct reVec3
{
	float x, y, z;
	reVec3(float x, float y, float z);
	reVec3();
};

struct reColor4
{
	float r, g, b, a;
	reColor4();
	reColor4(float r, float g, float b, float a);
};
*/
#endif // reTypes_h__