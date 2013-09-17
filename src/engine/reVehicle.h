#ifndef reVehicle_h__
#define reVehicle_h__

#include "rePlayer.h"

class btRaycastVehicle;
class reVehicleBody;

class reVehicle: public rePlayer
{
private:
	reVehicleBody* vehicleBody;
public:		
	reMat4 wheelTransforms[4];

	reVehicle();	
	void messageProcess( reMessageDispatcher* sender, reMessage* message );
	void runControl();
	void afterLoad();
	std::string className();
};

#endif // reVehicle_h__