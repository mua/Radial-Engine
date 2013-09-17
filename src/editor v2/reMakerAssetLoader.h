#ifndef reEditorAssetLoader_h__
#define reEditorAssetLoader_h__

#include "reAssetLoader.h"

class reMakerAssetLoader:
	public reAssetLoader
{
	void loadImage( std::string& fileName, reImage* out);
	virtual reObject* loadObject( reVar& var );
};

#endif // reEditorAssetLoader_h__