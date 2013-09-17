#ifndef reEditorAssetLoader_h__
#define reEditorAssetLoader_h__

#include "reAssetLoader.h"

class reEditorAssetLoader:
	public reAssetLoader
{
	void loadImage( std::string& fileName, reImage* out);
	virtual reObject* loadObject( reVar& json );
};

#endif // reEditorAssetLoader_h__