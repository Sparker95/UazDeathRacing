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
		Vehicle veh = Vehicle.Cast(newVehicleEntity);
		
		SCR_CompartmentAccessComponent compartmentAccessComponent = SCR_CompartmentAccessComponent.Cast(controlledEntity.FindComponent(SCR_CompartmentAccessComponent));
		compartmentAccessComponent.MoveInVehicle(veh, ECompartmentType.Pilot);
		
		CarControllerComponent m_pCarController = CarControllerComponent.Cast(veh.FindComponent(CarControllerComponent));
		m_pCarController.StartEngine();
    }
}