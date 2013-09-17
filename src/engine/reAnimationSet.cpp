#include "reAnimationSet.h"
#include "reNode.h"
#include <fstream>

#include "reRadial.h"
#include "reAssetLoader.h"
#include "reUtilities.h"

#include <iostream>
#include <glm/gtc/quaternion.hpp>

float reSequence::end() const
{
	return _end;
}

void reSequence::end( float val )
{
	_end = val;
}

float reSequence::start() const
{
	return _start;
}

void reSequence::start( float val )
{
	_start = val;
}

void reSequence::toJson( reVar& val, int categories /*= 1*/ )
{
	reObject::toJson(val, categories);
	val["start"] = start();
	val["end"] = end();
}

void reSequence::loadJson( reVar& value )
{
	reObject::loadJson(value);
	start(value["start"].asFloat());
	end(value["end"].asFloat());
}

std::string reSequence::className()
{
	return "reSequence";
}

//////////////////////////////////////////////////////////////////////////

rePose::rePose()
{
}

std::string rePose::nodeName() const
{
	return _nodeName;
}

void rePose::nodeName( std::string val )
{
	_nodeName = val;
}

void rePose::toVar( reVar& var )
{
	var["nodeName"] = nodeName();
}

void rePose::fromVar( reVar& var )
{
	nodeName(var["nodeName"].asString());
}

//////////////////////////////////////////////////////////////////////////

void reTransformPose::apply(reNode* node, float weight)
{	
	node->transform(glm::translate(reMat4(), translation) * glm::mat4_cast(glm::mix(rotation, reQuat(), weight)));
}

void reTransformPose::toVar( reVar& var )
{
	rePose::toVar(var);
	var["transform"] = transform().asVar();
}

void reTransformPose::fromVar( reVar& var )
{
	rePose::fromVar(var);
	reTransform t;
	t.fromVar(var["transform"]);
	transform(t);
	reMat4 rot;
	reVec3 scale;
	transform().decompose(translation, rot, scale);
	rotation = reQuat(rot);
}

reTransform reTransformPose::transform() const
{
	return _transform;
}

void reTransformPose::transform( reTransform val )
{
	_transform = val;
}

//////////////////////////////////////////////////////////////////////////

void reFrame::toVar( reVar& var )
{
	for (unsigned int i=0; i<poses.size(); i++)
	{
		poses[i]->toVar(var[i]);
	}
}

void reFrame::fromVar( reVar& var )
{
	for (unsigned int i=0; i<var.size(); i++)
	{
		reTransformPose* pose = new reTransformPose;
		pose->fromVar(var[i]);
		poses.push_back(pose);
	}
}

reFrame::~reFrame()
{
	for (size_t i=0; i<poses.size(); i++)
	{
		delete poses[i];
	}
}

reFrame::reFrame()
{

}

void reFrame::setPose( rePose* pose, const int index )
{
	if (index >= (int)poses.size())
	{
		poses.resize(index+1);
	}
	poses[index] = pose;
}

//////////////////////////////////////////////////////////////////////////

reAnimator::reAnimator()
{
	casted = false;	
	playing = false;
	_animset = 0;
}

reAnimator::~reAnimator()
{
	if (animset())
	{
		 reRadial::shared()->assetLoader()->releaseShared(animset());
	}
}

void reAnimator::toJson( reVar& val, int categories /*= 1*/ )
{
	if (!count())
	{
		reSequence* seq = new reSequence;
		seq->name("all");
		seq->start(0); seq->end(0);
		add(seq);
	}
	reCollection::toJson(val, categories);
	val["animsetFile"] = animsetFile();
}

void reAnimator::loadJson( reVar& value )
{
	reCollection::loadJson(value);
	animsetFile(value["animsetFile"].asString());
	if (animset())
	{
		castList = animset()->castList;		
	}
	casted = false;
}

std::string reAnimator::className()
{
	return "reAnimator";
}

reStringList reAnimator::acceptedClasses()
{
	reStringList ret;
	ret.push_back("reSequence");
	return ret;
}

void reAnimator::cast()
{
	for (size_t i=0; i<castList.size(); i++)
	{
		castList[i].node = dynamic_cast<reNode*>(super->objectByName(castList[i].nodeName, true));
	}
	casted = true;
}

void reAnimator::apply()
{
	if (!clips.size())
	{
		return;
	}
	if (!casted)
	{
		cast();
	}
	float total = 0.0f;
	for (int i=clips.size()-1; i>-1; i--)
	{
		clips[i]->act();
		if (clips[i]->state == reAnimationClip::stopped)
		{
			delete clips[i];
			clips.erase(clips.begin()+i);
			continue;
		}
		if (clips[i]->current)
		{
			total += clips[i]->weight;
		}		
	}
	for (size_t i=0; i<castList.size(); i++)
	{
		reNode* node = castList[i].node;
		reVec3 position(0,0,0);
		reQuat rotation(0,0,0,0);
		for (int n = 0; n<clips.size(); n++)
		{	
			std::cout << clips[n]->weight << std::endl;
			reFrame* current = clips[n]->current;
			reTransformPose* pose = (reTransformPose*)clips[n]->current->poses[i];
			float weight = clips[n]->weight / total;
			rotation = rotation + pose->rotation * weight;
			position = position + pose->translation * weight;
		}
		node->transform(glm::translate(reMat4(), position) * glm::mat4_cast(glm::normalize(rotation)));
	}
}

void reAnimator::play( reSequence* seq, float fadeIn )
{
	assert(seq);
	for ( size_t i=0; i<clips.size(); i++)
	{
		if (clips[i]->sequence->name() == seq->name())
		{
			return;
		}
	}
	reAnimationClip* clip = new reAnimationClip(seq, this);
	clips.push_back(clip);
	clip->play(fadeIn);
}

void reAnimator::play( std::string name, float fadeIn )
{
	play((reSequence*)objectByName(name));
}

void reAnimator::stopAnimation(std::string name, float fadeOut)
{
	for (int i=clips.size()-1; i>-1; i--)
	{
		if (name.empty() || name == clips[i]->sequence->name())
		{
			clips[i]->stop(fadeOut);
		}		
	}
}

void reAnimator::messageProcess( reMessageDispatcher* sender, reMessage* message )
{
	if (message->id = reM_TIMER)
	{
		apply();
	}	
}
int k=0;

void reAnimator::applyFrame( reFrame* frame )
{
	return;
	k++;
	frame = animset()->frames[2];
	reFrame* frame2 = animset()->frames[103];
	for (size_t i=0; i<frame->poses.size(); i++)
	{
		reNode* node = castList[i].node;
		reVec3 position;
		reQuat rotation;
		for (int n = 0; n<clips.size(); n++)
		{
			if (clips[n]->state == reAnimationClip::stopped) continue;
			reTransformPose* pose = (reTransformPose*)clips[n]->current->poses[i];
			rotation = rotation + pose->rotation * clips[n]->weight;
			position = position + pose->translation * clips[n]->weight;
		}
		node->transform(glm::translate(reMat4(), position) * glm::mat4_cast(glm::normalize(rotation)));
		//reTransformPose* pose2 = (reTransformPose*)frame2->poses[i];
		//frame2->poses[i]->apply(castList[i].node, 0);		
		//float weight = k%100/100.0f;
		//node->transform(glm::translate(reMat4(), pose->translation*weight+pose2->translation*(1-weight)) * glm::mat4_cast(glm::fastMix(pose->rotation, pose2->rotation, weight)));
		//node->transform(glm::translate(reMat4(), pose2->translation*(1-weight)+pose->translation*(weight)) * glm::mat4_cast(glm::normalize(pose->rotation * (1-weight) + pose2->rotation * weight)));
		/*
		node->transform(glm::translate(reMat4(), pose2->translation*weight) * glm::mat4_cast(glm::shortMix(reQuat(), pose2->rotation, weight)));
		weight = 1-weight;
		reVec3 t(node->transform().matrix[3][0], node->transform().matrix[3][1], node->transform().matrix[3][2]);
		node->transform(glm::translate(reMat4(), pose->translation*weight + t) * reMat4(reMat3(node->transform().matrix) * glm::mat3_cast(glm::shortMix(reQuat(), pose->rotation, weight))));
		*/
	}
	/*

	frame = animset()->frames[102];
	for (size_t i=0; i<frame->poses.size(); i++)
	{
		frame->poses[i]->apply(castList[i].node, 0);		
	}
	*/
}

std::string reAnimator::animsetFile() const
{
	return animset() ? animset()->path() : "";
}

void reAnimator::animsetFile( std::string val )
{
	animset(val.size() ? reRadial::shared()->assetLoader()->loadShared<reAnimset>(val) : 0);
}

reAnimset* reAnimator::animset() const
{
	return _animset;
}

void reAnimator::animset( reAnimset* val )
{
	_animset = val;
}

void reAnimator::initControl()
{
	
}

void reAnimator::pauseControl()
{
	playing = false;
	stopObserving(reRadial::shared());
}

void reAnimator::exitControl()
{
	stopObserving(reRadial::shared());
}

void reAnimator::runControl()
{
	observe(reRadial::shared(), reM_TIMER);
	playing = true;
}

reAnimationClip* reAnimator::clipByName( const std::string& name )
{
	for (int i=clips.size()-1; i>-1; i--)
	{
		if (name == clips[i]->sequence->name())
		{
			return clips[i];
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////

reAnimationClip::reAnimationClip( reSequence* seq, reAnimator* set )
{
	startTime = -1;
	sequence = seq;
	animator = set;
	weight = 1;
	current = set->animset()->frames[seq->start()];
	state = stopped;
	fadeDelta = 0;
}

void reAnimationClip::play(float fadeIn)
{
	if (state == starting || state == playing)
	{
		return;
	}
	startTime = reRadial::shared()->time();
	fadeTime = startTime;
	state = starting;
	fadeDelta = 1/fadeIn;
}

void reAnimationClip::stop(float fadeOut)
{
	if (state == stopping || state == stopped)
	{
		return;
	}
	fadeTime = reRadial::shared()->time();
	fadeDelta = 1/fadeOut;
	state = stopping;
}

void reAnimationClip::apply( float weight/*=1*/ )
{
	//std::cout << reRadial::shared()->time() << std::endl;
	float deltaTime = (reRadial::shared()->time() - startTime);
	weight = max(min(weight + fadeDelta * deltaTime, 1), 0);
	int delta = int(deltaTime * 30);
	int startFrame = sequence->start();
	int endFrame = sequence->end() ? sequence->end() : animator->animset()->frames.size()-1;
	int frame = startFrame + (delta % (endFrame - startFrame));
	animator->applyFrame(animator->animset()->frames[frame]);
}

void reAnimationClip::act()
{	
	if (state & (starting | stopping))
	{
		float fadeDeltaTime = (reRadial::shared()->time() - fadeTime);
		switch (state)
		{
		case starting:
			weight = min(fadeDelta * fadeDeltaTime, 1.0f);
			if (weight == 1.0f) 
			{
					state = playing;
					startTime = reRadial::shared()->time();
			}
			return;
			break;
		case stopping:
			weight = max(1.0f - fadeDelta * fadeDeltaTime, 0.0f);
			if (weight == 0.0f) state = stopped;
			break;
		}		
		//return;
	}

	float deltaTime = (reRadial::shared()->time() - startTime);
	int delta = int(deltaTime * 30);
	int startFrame = sequence->start();
	int endFrame = sequence->end() ? sequence->end() : animator->animset()->frames.size()-1;
	int frame = startFrame + (delta % (endFrame - startFrame));
	current = animator->animset()->frames[frame];
}

//////////////////////////////////////////////////////////////////////////

reCast::reCast( reNode* _node ) :
node(_node), nodeName(_node->name())
{

}

bool reCast::operator==( const reCast& other ) const
{
	return other.node == node;
}

//////////////////////////////////////////////////////////////////////////

reAnimset::~reAnimset()
{
	for (size_t i=0; i<frames.size(); i++)
	{
		delete frames[i];
	}
}

void reAnimset::toJson( reVar& val, int categories /*= 1*/ )
{
	reCollection::toJson(val, categories);
	reVar& framesVal = val["frames"];
	for ( size_t i=0; i<frames.size(); i++)
	{
		frames[i]->toVar(framesVal[i]);
	}
	for (size_t i=0; i<castList.size(); i++)
	{
		val["cast"][i] = castList[i].nodeName;
	}
}

void reAnimset::loadJson( reVar& value )
{
	reCollection::loadJson(value);
	for (int i=0; i<value["cast"].size(); i++)
	{
		reCast cast;
		cast.nodeName = value["cast"][i].asString();
		castList.push_back(cast);
	}
	reVar& framesVal = value["frames"];
	for ( size_t i=0; i<framesVal.size(); i++)
	{
		reFrame* frame = new reFrame;
		frame->fromVar(framesVal[i]);
		frames.push_back(frame);
	}
}

reFrame* reAnimset::getFrame( int i )
{
	if (i>=frames.size())
	{
		frames.resize(i+1);
		frames[i] = new reFrame;
	}
	return frames[i];
}

void reAnimset::addPose( int frameIndex, reNode* node, rePose* pose )
{
	reCast cast(node);
	int castIndex = find(castList.begin(), castList.end(), cast) - castList.begin();
	if (castIndex == (castList.end() - castList.begin()))
	{
		castList.push_back(node);
		castIndex = castList.size()-1;
	}
	reFrame* frame = getFrame(frameIndex);
	frame->setPose(pose, castIndex);
}

std::string reAnimset::className()
{
	return "reAnimset";
}
