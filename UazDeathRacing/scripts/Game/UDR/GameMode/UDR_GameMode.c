class UDR_GameModeClass: SCR_BaseGameModeClass
{
};

enum ERaceState
{
	WAITING_PLAYERS,
	COUNTDOWN,
	RACING,
	FINISH_SCREEN
}

class UDR_GameMode: SCR_BaseGameMode
{
	// Constants
	protected const float RACE_TRACK_LOGIC_UPDATE_INTERVAL = 0.25;
	
	
	// Other entities for the game mode
	[Attribute("", UIWidgets.EditBox)]
	protected string m_sRaceTrackLogicEntity;
	[Attribute("", UIWidgets.EditBox)]
	protected string m_sVehiclePositioningEntity;
	
	protected UDR_RaceTrackLogicComponent m_RaceTrackLogic;
	protected UDR_VehiclePositioning m_VehiclePositioning;
	
	// State of the race
	[RplProp()]
	protected ERaceState m_RaceState = ERaceState.WAITING_PLAYERS;

	// IDs of players which are assigned to the race
	protected ref array<UDR_PlayerNetworkComponent> m_aRacers = {};
		
	//-------------------------------------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
			
		IEntity raceTrackLogicEnt = GetGame().FindEntity(m_sRaceTrackLogicEntity);
		if (raceTrackLogicEnt)
			m_RaceTrackLogic = UDR_RaceTrackLogicComponent.Cast(raceTrackLogicEnt.FindComponent(UDR_RaceTrackLogicComponent));
		if (!m_RaceTrackLogic)
			Print("Could not find UDR_RaceTrackLogicComponent!", LogLevel.ERROR);
		
		IEntity vehiclePosEnt = GetGame().FindEntity(m_sVehiclePositioningEntity);
		if (vehiclePosEnt)
			m_VehiclePositioning = UDR_VehiclePositioning.Cast(vehiclePosEnt);
		if (!m_VehiclePositioning)
			Print("Could not find UDR_VehiclePositioning entity!", LogLevel.ERROR);
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
    override void OnPlayerSpawned(int playerId, IEntity controlledEntity)
	{
		super.OnPlayerSpawned(playerId, controlledEntity);

		vector playerPosition[4];
		controlledEntity.GetWorldTransform(playerPosition);
		
		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);
		UDR_PlayerNetworkComponent playerNetworkComp = UDR_PlayerNetworkComponent.Cast(pc.FindComponent(UDR_PlayerNetworkComponent));
		
        // List of UDR custom vehicles
		array<ResourceName> vehiclePrefabs = {
			"{1A20D130A03F9CF1}Prefabs/Vehicles/Wheeled/UAZ469/UAZ469_Armed.et",
			"{21C45FA677BCDBDA}Prefabs/Vehicles/Wheeled/M998/M998_Armed.et"
		};

		// spawn vehicle
		Resource res = Resource.Load(vehiclePrefabs[0]);
		IEntity newVehicleEntity = GetGame().SpawnEntityPrefab(res, params: (new EntitySpawnParams));
		newVehicleEntity.SetWorldTransform(playerPosition);
		
		// Assign vehicle to player
		playerNetworkComp.m_AssignedVehicle = newVehicleEntity;
		
		// Register the vehicle to race track logic
		m_RaceTrackLogic.RegisterRacer(newVehicleEntity);
		
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
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void GetPlayerNetworkComponents(notnull array<UDR_PlayerNetworkComponent> outComponents)
	{
		PlayerManager pm = GetGame().GetPlayerManager();
		array<int> playerIds = {};
		pm.GetPlayers(playerIds);
		
		foreach (int id : playerIds)
		{
			PlayerController pc = pm.GetPlayerController(id);
			if (!pc)
				continue;
			UDR_PlayerNetworkComponent playerComp = UDR_PlayerNetworkComponent.Cast(pc.FindComponent(UDR_PlayerNetworkComponent));
			if (!playerComp)
				continue;
			
			outComponents.Insert(playerComp);
		}
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	float m_fUpdateRaceTrackLogicTimer = RACE_TRACK_LOGIC_UPDATE_INTERVAL;
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);
		
		
		// Update the race track logic based on timer (no need to update too often)
		m_fUpdateRaceTrackLogicTimer += timeSlice;
		if (m_fUpdateRaceTrackLogicTimer >= RACE_TRACK_LOGIC_UPDATE_INTERVAL)
		{
			m_RaceTrackLogic.UpdateAllRacers();
			
			// Update player component of each player
			PlayerManager playerMgr = GetGame().GetPlayerManager();
			array<int> players = {};
			playerMgr.GetPlayers(players);
			
			array<UDR_PlayerNetworkComponent> playerComponents = {};
			foreach (int playerId : players)
			{
				PlayerController playerController = playerMgr.GetPlayerController(playerId);
				if (!playerController)
					continue;
				UDR_PlayerNetworkComponent playerComp = UDR_PlayerNetworkComponent.Cast(playerController.FindComponent(UDR_PlayerNetworkComponent));
				if (!playerComp)
					continue;
				
				IEntity assignedVehicle = playerComp.m_AssignedVehicle;
				if (!assignedVehicle)
					continue;
				
				// Get race track data of the player
				float totalProgress;
				int lapCount;
				int nextWaypoint;
				if (!m_RaceTrackLogic.GetRacerData(assignedVehicle, totalProgress, lapCount, nextWaypoint))
					continue;
				
				playerComp.m_iLapCount = lapCount;
				playerComp.m_iNextWaypoint = nextWaypoint;
				playerComp.m_fTotalProgress = totalProgress;
				
				playerComponents.Insert(playerComp);
			}
			
			// Sort player components by total track progress, to find who is first, second, etc
			SCR_Sorting<UDR_PlayerNetworkComponent, UDR_PlayerNetworkComponent_CompareTotalProgress>.HeapSort(playerComponents, true);
			
			foreach (int i, UDR_PlayerNetworkComponent playerComp : playerComponents)
			{
				playerComp.m_iPositionInRace = i;
				playerComp.BumpReplication(); // Finally, replicate this
			}
			
			m_fUpdateRaceTrackLogicTimer -= RACE_TRACK_LOGIC_UPDATE_INTERVAL;
		}
		
		UpdateRaceState();
	}
	
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void UpdateRaceState(float timeSlice)
	{
		switch (m_RaceState)
		{
			case ERaceState.WAITING_PLAYERS:
			{
			}
			
			case ERaceState.COUNTDOWN:
			{
			}
			
			case ERaceState.RACING:
			{
			}
			
			case ERaceState.FINISH_SCREEN:
			{
			}
		}
	}
}