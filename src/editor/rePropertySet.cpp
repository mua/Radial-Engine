#include "rePropertySet.h"
#include "rePropertyEditor.h"

//////////////////////////////////////////////////////////////////////////

rePropertySet::rePropertySet()
{
	parent = 0;
}

void rePropertySet::addProperty( rePropertyBase* property )
{
	if (!properties.size())
	{
		createSubsets(property);
		name = property->name;
	}
	properties.push_back(property);
	for (int i=0; i<property->subProperties.size(); i++)
	{
		subsets[i]->addProperty(property->subProperties[i]);
	}
}

void rePropertySet::createSubsets( rePropertyBase* property )
{
	for (unsigned int i=0; i<property->subProperties.size(); i++)
	{
		rePropertySet* pset = new rePropertySet;
		pset->parent = this;
		subsets.push_back(pset);
	}
}

void rePropertySet::set( QVariant val )
{
	for (rePropertyContainer::iterator it=properties.begin(); it!=properties.end(); it++)
	{
		(*it)->setVariant(val);
	}
}

QVariant rePropertySet::get()
{
	QVariant ret;
	for (rePropertyContainer::iterator it=properties.begin(); it!=properties.end(); it++)
	{
		if (it==properties.begin())
		{
			ret = (*it)->getVariant();
		} else if (ret != (*it)->getVariant());
		//ret = 1; // Multiple
	}
	return ret;
}

void rePropertySet::updateEditor()
{
	/*
	if (property)
	{
		property->setValue(get());
	}	
	for (unsigned int i=0; i<subsets.size(); i++)
	{
		subsets[i]->updateEditor();
	}
	*/
}

void rePropertySet::saveEditor()
{

}

bool rePropertySet::editable()
{
	return properties.size() && properties[0]->enabled();
}