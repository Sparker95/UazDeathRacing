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

		// spawn vehicle
		Resource res = Resource.Load(vehiclePrefabs[0]);
		IEntity newVehicleEntity = GetGame().SpawnEntityPrefab(res, params: (new EntitySpawnParams));
		newVehicleEntity.SetWorldTransform(playerPosition);
		
		// register to destroyed event
		EventHandlerManagerComponent ev = EventHandlerManagerComponent.Cast(newVehicleEntity.FindComponent(EventHandlerManagerComponent));
        ev.RegisterScriptHandler("OnDestroyed", newVehicleEntity, OnVehicleDestroyed);
		
		// save some datas in vehicleComp
		Vehicle vehicule = Vehicle.Cast(newVehicleEntity);
		UDR_VehicleNetworkComponent vehNetComp = UDR_VehicleNetworkComponent.Cast(vehicule.FindComponent(UDR_VehicleNetworkComponent));
		vehNetComp.SetPlayerControllerID(playerId);

		// move player in pilot and start the car
		SCR_CompartmentAccessComponent compartmentAccessComponent = SCR_CompartmentAccessComponent.Cast(controlledEntity.FindComponent(SCR_CompartmentAccessComponent));
		compartmentAccessComponent.MoveInVehicle(newVehicleEntity, ECompartmentType.Pilot);
		CarControllerComponent m_pCarController = CarControllerComponent.Cast(vehicule.FindComponent(CarControllerComponent));
		m_pCarController.StartEngine();
		
		SCR_DamageManagerComponent damageManager = SCR_DamageManagerComponent.Cast(controlledEntity.FindComponent(SCR_DamageManagerComponent));
		if (damageManager) {
			damageManager.EnableDamageHandling(false);
		}
    }
	
	void OnVehicleDestroyed(IEntity vecEntity)
	{
		int playerID = UDR_VehicleNetworkComponent.Cast(vecEntity.FindComponent(UDR_VehicleNetworkComponent)).GetPlayerControllerID();
		if (!playerID) {
			Print("no player found attached to this vehicle");
			return;
		}
		
		GetGame().GetCallqueue().CallLater(ForceRespawnPlayer, 25000, false, playerID);
	}

	void ForceRespawnPlayer(int playerID)
	{
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerID);
		if (!playerController) {
			PrintFormat("no PlayerController found for playerID: %1", playerID);
			return;
		}
		
		IEntity playerEntity = playerController.GetControlledEntity();
		if (!playerEntity) {
			PrintFormat("no IEntity found for playerID: %1", playerID);
			return;
		}
		
		SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(playerEntity.FindComponent(SCR_CharacterControllerComponent));
		if (characterController) {
			characterController.ForceDeath();
			PrintFormat("ForceDeath playerID: %1", playerID);
		}
	}
}