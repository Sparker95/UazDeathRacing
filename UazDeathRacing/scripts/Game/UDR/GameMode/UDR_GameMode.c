class UDR_GameModeClass: SCR_BaseGameModeClass
{
};

class UDR_GameMode: SCR_BaseGameMode
{
    override void OnPlayerSpawned(int playerId, IEntity controlledEntity)
	{
		super.OnPlayerSpawned(playerId, controlledEntity);
		
		vector playerPosition[4];
		controlledEntity.GetWorldTransform(playerPosition);
		
        // List of UDR custom vehicles
		array<ResourceName> vehiclePrefabs = {
			"{1A20D130A03F9CF1}Prefabs/Vehicles/Wheeled/UAZ469/UAZ469_Armed.et",
			"{21C45FA677BCDBDA}Prefabs/Vehicles/Wheeled/M998/M998_Armed.et"
		};

		Resource res = Resource.Load(vehiclePrefabs[0]);
		EntitySpawnParams spawnParams = new EntitySpawnParams();
		IEntity newVehicleEntity = GetGame().SpawnEntityPrefab(res, params: spawnParams);
		newVehicleEntity.SetWorldTransform(playerPosition);
		newVehicleEntity.SetName("Vehicle_"+playerId);
		
		Vehicle veh = Vehicle.Cast(newVehicleEntity);
		
		SCR_CompartmentAccessComponent compartmentAccessComponent = SCR_CompartmentAccessComponent.Cast(controlledEntity.FindComponent(SCR_CompartmentAccessComponent));
		compartmentAccessComponent.MoveInVehicle(veh, ECompartmentType.Pilot);
		
		CarControllerComponent m_pCarController = CarControllerComponent.Cast(veh.FindComponent(CarControllerComponent));
		m_pCarController.StartEngine();
		
		array<SCR_ParticleEmitter> emittersList = findEmitters(newVehicleEntity);
		foreach (SCR_ParticleEmitter emitter : emittersList)
			emitter.Stop();
    }
	
	array<SCR_ParticleEmitter> findEmitters(IEntity vehicle) 
	{
		array<SCR_ParticleEmitter> emitters = new array<SCR_ParticleEmitter>;
		IEntity child = vehicle.GetChildren();
		while (child)
		{
			SCR_ParticleEmitter childSlot = SCR_ParticleEmitter.Cast(child);
			if (childSlot)
				if (childSlot.GetPathToPTC() == UDR_BoostVehicleTrigger.BOOST_PARTICLE_NAME)
					emitters.Insert(childSlot);
			
			child = child.GetSibling();
		}
		
		return emitters;
	}
}