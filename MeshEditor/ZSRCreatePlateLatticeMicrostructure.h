#pragma once
#include "ZSRACISHeads.h"

class ZSRCreatePlateLatticeMicrostructure
{
public:
	ZSRCreatePlateLatticeMicrostructure(void);
	~ZSRCreatePlateLatticeMicrostructure(void);

	ZSRCreatePlateLatticeMicrostructure(double *piDensity);

	BODY* CreateCube(const double diLength);
	ENTITY_LIST CreateKeyFace(ENTITY_LIST );

	
};

