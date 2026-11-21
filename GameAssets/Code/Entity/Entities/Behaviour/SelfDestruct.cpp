#include "SelfDestruct.h"

SelfDestruct::SelfDestruct(unsigned int ticks) 
{
	this->ticksLeft = ticks;
}
void SelfDestruct::update() 
{
	if(ticksLeft) ticksLeft--;
	else this->getParent()->destroy();
}