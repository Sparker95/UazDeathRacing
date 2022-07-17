class UDR_GameModeClass: SCR_BaseGameModeClass
{
};

class UDR_GameMode: SCR_BaseGameMode
{
	[Attribute("", UIWidgets.EditBox)]
	protected string m_sRaceTrackLogicEntity;
	
	protected UDR_RaceTrackLogicComponent m_RaceTrackLogic;
	
	//-------------------------------------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
			
		IEntity raceTrackLogicEnt = GetGame().FindEntity(m_sRaceTrackLogicEntity);
		if (raceTrackLogicEnt)
			m_RaceTrackLogic = UDR_RaceTrackLogicComponent.Cast(raceTrackLogicEnt.FindComponent(UDR_RaceTrackLogicComponent));
		
		if (!m_RaceTrackLogic)
		{
			Print("Could not find UDR_RaceTrackLogicComponent!");
		}
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
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
		
		// Register the vehicle to race track logic
		m_RaceTrackLogic.RegisterRacer(newVehicleEntity, 1);
		
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
	
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void OnVehicleDestroyed(IEntity vehEntity)
	{
		m_RaceTrackLogic.UnregisterRacer(vehEntity);
		
		int playerID = UDR_VehicleNetworkComponent.Cast(vehEntity.FindComponent(UDR_VehicleNetworkComponent)).GetPlayerControllerID();
		if (!playerID) {
			Print("no player found attached to this vehicle");
			return;
		}
		
		// TODO: handle the case of being run by which will auto respawn and then die again with this
		GetGame().GetCallqueue().CallLater(ForceRespawnPlayer, 5000, false, playerID);
	}
	
	
	//-------------------------------------------------------------------------------------------------------------------------------
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
	
	
	//-------------------------------------------------------------------------------------------------------------------------------
	// Returns Player ID of player assigned to this vehicle
	int GetVehiclePlayerId(Vehicle veh)
	{
		UDR_VehicleNetworkComponent vehNetworkComp = UDR_VehicleNetworkComponent.Cast(veh.FindComponent(UDR_VehicleNetworkComponent));
		if (!vehNetworkComp)
			return -1;
		
		return vehNetworkComp.GetPlayerControllerID();
	}
}