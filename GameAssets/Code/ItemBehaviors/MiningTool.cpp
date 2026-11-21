#include "MiningTool.h"
#include "System/Input.h"
#include "VoxelMaterials.h"
#include "GameNode/ComponentFactory.h"
#include "VoxelData.h"
#include "GameNode/Properties/ComponentRefProp.h"
#include "Core.h"
#include "GameNode/Properties/GameNodeProp.h"


MiningTool::MiningTool() 
{
	reach = 8.0f;
	radius = 3.0f;
	heatingMaterial = nullptr;
	this->addProp(new StringKeyMapValuePointerProp<Kirino::Material>(&heatingMaterial, &Core::materialsManager.getLoadedMap()), "Heating material");
	this->addProp(new IntProp(&breakCD), "break cd");
	this->addProp(new FloatProp(&reach), "reach");
	this->addProp(new FloatProp(&radius), "radius");
	this->addProp(new FloatProp(&heatDissipation), "Heat dissipation");
	this->addProp(new FloatProp(&heatingSpeed), "Heating");
	this->addProp(new FloatProp(&laserBeamChargeSpeed), "Laser beam charge speed");
	this->addProp(new GameNodeProp(&beamHeatupModel), "Beam heatup model");
	this->addProp(new GameNodeProp(&beamModel), "Beam model");
	heatingSpeed = 0.004f;
	heatDissipation = 0.5f;
	heat = 0.0f;
	beamHeatupModel = nullptr;
	beamModel = nullptr;
	laserBeamCharge = 0.0f;
	laserBeamChargeSpeed = 0.01f;
}

void MiningTool::onUseHoldTick(unsigned char usage) 
{
	if(usage != ItemUsageType::primary) return;
	laserBeamCharge += laserBeamChargeSpeed;
	if(laserBeamCharge >= 1.0f) laserBeamCharge = 1.001f;
}

void MiningTool::updateItem() 
{
	bool breakCondition = true;
	bool hasLaserBeam = false;
	if(beamHeatupModel) ((GameNode3D*)beamHeatupModel)->getRelativeTransform().setScale(glm::vec3(0.0f));
	if(beamModel) ((GameNode3D*)beamModel)->getRelativeTransform().setScale(glm::vec3(0.0f));

	if(!isUsageHeld(ItemUsageType::primary))
	{
		laserBeamCharge -= laserBeamChargeSpeed * 2.0f;
		if(laserBeamCharge < 0.0f) laserBeamCharge = 0.0f;
		heat -= 0.001f;
		heat *= glm::mix(1.0f, heat, heatDissipation);
		breakCDTimer = 0;
	}
	if(laserBeamCharge < 1.0f) breakCondition = false;
	if(laserBeamCharge >= 1.0f) 
	{
		hasLaserBeam = true;
	}
	
	if(hasLaserBeam) 
	{
		breakCDTimer += breakCD;
		if(breakCDTimer < breakCD) breakCondition = false;
		else breakCDTimer -= breakCD;
		heat += heatingSpeed;
		if(heat > 1.0f) heat = 1.0f;
		const Transform& t = this->getParentOfType<GameNode3D>()->getWorldTransform();
		glm::vec3 p = t.getPosition();
		glm::vec3 f = t.forward();
		glm::ivec3 hitIndex;
		glm::vec3 normal;
		
		bool hit = World::currentWorld->rayCast(p, f, 64, reach, hitIndex, normal);
		
		if(beamModel) 
		{
			Transform& t = ((GameNode3D*)beamModel)->getRelativeTransform();
			t.setScale(glm::vec3(1.0f, 1.0f, glm::distance(((GameNode3D*)beamModel)->getWorldTransform().getPosition(), p)));
			t.lookAt(((GameNode3D*)beamModel)->getWorldTransform().getPosition() - p, glm::vec3(0.0f, 1.0f, 0.0f));
		}
		if(Core::client.isConnected() == true) breakCondition = false;
		if(hit && breakCondition)
		{
			for(int i = ((int)glm::floor(-radius)) - 1; i <= ((int)glm::floor(radius)) + 2; i++)
			for(int j = ((int)glm::floor(-radius)) - 1; j <= ((int)glm::floor(radius)) + 2; j++)
			for(int k = ((int)glm::floor(-radius)) - 1; k <= ((int)glm::floor(radius)) + 2; k++) 
			{
				glm::ivec3 vIndex = hitIndex + glm::ivec3(i,j,k);
				glm::vec3 vPos = glm::vec3(vIndex);
				float r = glm::length(vPos - p);
				float val = glm::clamp(-1.0f + r / (float)(radius), -1.0f, 1.0f);
				Voxel v = World::currentWorld->getVoxel(vIndex);
				short oAmount = v.amount;
				float oAmountF = float(v.amount) / voxelAmountScale;
				v.amount = glm::min(v.amount, (signed char)((glm::mix(oAmountF, val, 0.1f)) * voxelAmountScale));
				World::currentWorld->setVoxel(glm::vec3(vIndex), v);
				if(oAmount > 0 && v.amount <= 0) 
				{
					VoxelMaterials::getInstance().getById(v.voxelTypeId).breakEvent->invoke(vIndex, VoxelMaterials::getInstance().getById(v.voxelTypeId));
				}
			}
		}
	}
	else 
	{
		if(beamHeatupModel) ((GameNode3D*)beamHeatupModel)->getRelativeTransform().setScale(glm::vec3(laserBeamCharge * 0.5f));
	}
	if(heatingMaterial) 
	{
		glm::vec4 ucolor = glm::vec4(heat);
		const_cast<Kirino::Material*>(heatingMaterial)->setUniform("unlitColor", &ucolor, sizeof(float) * 4, 0);
	}
}

EXPORT_COMPONENT(MiningTool);
