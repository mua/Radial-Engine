#ifndef rePropertySet_h__
#define rePropertySet_h__

#include "reTypes.h"
#include "reTransform.h"
#include "QDebug"

#include <QtCore>
#include <vector>

using namespace std;

class reNodeEditor;

class rePropertyBase
{
public:
	QString name;	
	vector<rePropertyBase*> subProperties;
	void addSubProperty(rePropertyBase* property)
	{
		subProperties.push_back(property);
	}
	virtual QVariant getVariant() {return 0;};
	virtual void setVariant(QVariant value) {};
	virtual bool enabled() { return true; };
};

template <typename T, typename V, void(T::*SET)(V), V (T::*GET)()const>
class reProperty: public rePropertyBase
{
public:
	typedef V ValueType;
	typedef T HostType;
	T* host;
	reProperty() {};
	void setHost(T* host) {this->host = host;};
	void set(V val) { (host->*SET)(val); };
	V get() { return (host->*GET)(); };
};

template <typename T, void(T::*SET)(float), float(T::*GET)()const>
class reScalarProperty: public reProperty<T, float, SET, GET>
{
	virtual QVariant getVariant() {float v = (host->*GET)(); return (double)(abs(v) > 0.001f ? v : 0.0f);};
	virtual void setVariant(QVariant value) { (host->*SET)(value.toFloat()); };
};

template <typename T, void(T::*SET)(reVec3), reVec3 (T::*GET)()const>
class reVec3Property: public reProperty<T, reVec3, SET, GET>
{
public:
	void x(float _x) { reVec3 v = (host->*GET)(); v.x = _x; (host->*SET)(v);};
	float x() const { return (host->*GET)().x;};
	void y(float _y) { reVec3 v = (host->*GET)(); v.y = _y; (host->*SET)(v);};
	float y() const { return (host->*GET)().y;};
	void z(float _z) { reVec3 v = (host->*GET)(); v.z = _z; (host->*SET)(v);};
	float z() const { return (host->*GET)().z;};

	reScalarProperty<reVec3Property, &reVec3Property::x, &reVec3Property::x> xProperty;
	reScalarProperty<reVec3Property, &reVec3Property::y, &reVec3Property::y> yProperty;
	reScalarProperty<reVec3Property, &reVec3Property::z, &reVec3Property::z> zProperty;

	reVec3Property ()
	{
		xProperty.setHost(this); xProperty.name = "x";
		yProperty.setHost(this); yProperty.name = "y";
		zProperty.setHost(this); zProperty.name = "z";
		addSubProperty(&xProperty);
		addSubProperty(&yProperty);
		addSubProperty(&zProperty);
	};

	virtual QVariant getVariant() 
	{
		return QString("%1, %2, %3").arg(x()).arg(y()).arg(z());
	}

	virtual void setVariant(QVariant value)
	{
		// x(value.toFloat());
	}
	virtual bool enabled() { return false; };
};

template <typename T, void(T::*SET)(reTransform), reTransform (T::*GET)()const>
class reTransformProperty: public reProperty<T, reTransform, SET, GET>
{
public:
	void position(reVec3 pos) 
	{ 
		reTransform t = (host->*GET)();
		reVec3 delta = pos - position();
		if (glm::length(delta) < 0.01)
		{
			return;
		}
		t.matrix = glm::translate(reMat4(), delta) * t.matrix;
		(host->*SET)(t);
	};
	reVec3 position() const { 
		reVec3 v((host->*GET)().matrix * reVec4(0,0,0,1) );
		return v;
	};

	void rotation(reVec3 pos) 
	{ 
		reTransform t = (host->*GET)(); 
		reVec3 delta = pos - rotation();
		reMat4 rot = 
			glm::rotate(reMat4(), delta[0], reVec3(1, 0, 0)) *
			glm::rotate(reMat4(), delta[1], reVec3(0, 1, 0)) *			
			glm::rotate(reMat4(), delta[2], reVec3(0, 0, 1)) *
			reMat4();
		t.matrix = glm::translate(reMat4(), position()) * rot * glm::translate(reMat4(), -position()) * t.matrix;
		(host->*SET)(t);
	};
	reVec3 rotation () const 
	{
		reMat4 &r = (host->*GET)().matrix;
		double thetaX, thetaY, thetaZ;
		double pi = M_PI;

		if (r[0][2] < +1)
		{
			if (r[0][2] > -1)
			{
				thetaY = asin(-r[0][2]);
				thetaX = atan2(r[1][2],r[2][2]);
				thetaZ = atan2(r[0][1],r[0][0]);
			}
			else // r02 = -1
			{
				// Not a unique solution: thetaZ - thetaY = atan2(-r01,r00)
				thetaY = -pi/2;
				thetaX = -atan2(-r[1][0],r[1][1]);
				thetaZ = 0;
			}
		}
		else // r12 = +1
		{
			// Not a unique solution: thetaZ + thetaY = atan2(-r01,r00)
			thetaY = pi/2;
			thetaX = atan2(-r[1][0],r[1][1]); 
			thetaZ = 0;
		}
		/*
		float z = glm::degrees(glm::atan(m[0][1]/m[1][1]));
		float x = glm::degrees(glm::asin(-m[2][1]));
		float y = glm::degrees(glm::atan(m[2][0]/m[2][2]));
		*/
		float z = (float)glm::degrees(thetaZ);
		float x = (float)glm::degrees(thetaX);
		float y = (float)glm::degrees(thetaY);

		x = glm::isnan(x) ? 0 : x;
		y = glm::isnan(y) ? 0 : y;
		z = glm::isnan(z) ? 0 : z;
		return reVec3(x,y,z);
	}

	reVec3Property<reTransformProperty, &reTransformProperty::rotation, &reTransformProperty::rotation> rotationProperty;
	reVec3Property<reTransformProperty, &reTransformProperty::position, &reTransformProperty::position> positionProperty;

	reTransformProperty() 
	{
		name = "Transform";
		rotationProperty.name = "Rotation";
		positionProperty.name = "Position";
		rotationProperty.setHost(this);
		positionProperty.setHost(this);
		addSubProperty(&rotationProperty);
		addSubProperty(&positionProperty);
	}
	virtual QVariant getVariant() 
	{
		return QVariant(false); //QString("%1, %2, %3").arg(position()[0]).arg(position()[1]).arg(position()[2]);
	}
	virtual void setVariant(QVariant value)
	{
		//x(value);
	}
	virtual bool enabled() { return false; };
};

class rePropertySet;
class reNodeEditor;

typedef vector<rePropertyBase*> rePropertyContainer;
typedef vector<rePropertySet*> rePropertySetContainer;

class rePropertySet
{
public:
	rePropertyContainer properties;
	rePropertySetContainer subsets;
	rePropertySet* parent;
	QString name;

	rePropertySet();
	QVariant get();
	void set(QVariant val);
	void createSubsets(rePropertyBase* property);
	void addProperty(rePropertyBase* property);
	void updateEditor();
	void saveEditor();
	bool editable();
};
#endif // rePropertySet_h__
