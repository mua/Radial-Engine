#include "reCollection.h"
#include "reTypes.h"
#include "reTransform.h"
#include "reVar.h"
#include "reMessage.h"
#include "reController.h"

#include <vector>

class reNode;
class reAnimator;

class rePose
{
private:
	std::string _nodeName;
public:
	rePose();
	virtual void apply(reNode* node, float weight) = 0;
	virtual void toVar(reVar& var);
	virtual void fromVar(reVar& var);
	std::string nodeName() const;
	void nodeName(std::string val);
};

class reTransformPose: public rePose
{
private:
	reTransform _transform;
public:
	reQuat rotation;
	reVec3 translation;
	virtual void apply(reNode* node, float weight);
	virtual void toVar(reVar& var);
	virtual void fromVar(reVar& var);
	reTransform transform() const;
	void transform(reTransform val);
};

typedef std::vector<rePose*> rePoseList;

class reFrame
{
public:
	rePoseList poses;
	~reFrame();
	reFrame();
	void toVar(reVar& var);
	void fromVar(reVar& var);
	void setPose( rePose* pose, const int order );
};

typedef std::vector<reFrame*> reFrameList;

class reSequence: public reObject
{
private:
	float _start, _end;
public:	
	float end() const;
	void end(float val);
	float start() const;
	void start(float val);
	virtual void toJson(reVar& val, int categories = 1) override;
	virtual void loadJson(reVar& value) override;
	virtual std::string className() override;
};

class reAnimationClip
{
public:
	float fadeTime;
	float weight;
	float fadeDelta;
	typedef enum 
	{
		starting = 1,
		playing = 2,
		stopping = 4,
		stopped = 8,
	} reClipState;

	reClipState state;

	reSequence* sequence;
	reAnimator* animator;
	float startTime;
	reFrame* current;
	reAnimationClip(reSequence* seq = 0, reAnimator* set=0);
	void play(float fadeIn = 0);
	void stop(float fadeOut = 0);
	void act();
	void apply(float weight=1);
};

typedef std::vector<reAnimationClip*> reClipList;

struct reCast
{
	std::string nodeName;
	reNode* node;
	reCast()
	{
		node = 0;
	}
	reCast(reNode* _node);
	bool operator==(const reCast& other) const;
};

typedef std::vector<reCast> reCastList;

class reAnimset: public reTypedCollection<reSequence>
{
public:
	~reAnimset();
	reCastList castList;
	reFrameList frames;
	virtual void toJson(reVar& val, int categories = 1) override;
	virtual void loadJson(reVar& value) override;	
	reFrame* getFrame( int i );
	void addPose( int frameIndex, reNode* node, rePose* pose );
	virtual std::string className() override;
};

class reAnimator: public reTypedCollection<reSequence>, public reMessageObserver, public reController
{
private:
	reAnimset* _animset;
	bool playing;	
public:
	reClipList clips;
	reCastList castList;
	bool casted;	

	reAnimator();
	~reAnimator();	

	virtual std::string className() override;
	virtual reStringList acceptedClasses() override;
	virtual void toJson(reVar& val, int categories = 1) override;
	virtual void loadJson(reVar& value) override;

	void play(std::string name, float fadeIn = 0);
	void play(reSequence* seq, float fadeIn = 0);
	void stopAnimation(std::string name, float fadeOut = 0);
	void apply();
	void cast();

	virtual void messageProcess(reMessageDispatcher* sender, reMessage* message) override;
	void applyFrame( reFrame* frame );

	std::string animsetFile() const;
	void animsetFile(std::string val);
	reAnimset* animset() const;
	void animset(reAnimset* val);
	reAnimationClip* clipByName(const std::string& name);

	virtual void initControl();
	virtual void pauseControl();
	virtual void exitControl();
	virtual void runControl();
	
};
