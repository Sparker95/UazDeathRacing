class UDR_GameModeClass: SCR_BaseGameModeClass
{
};

enum ERaceState
{
	NO_PLAYERS,			// Initial state until someone connects
	ONE_PLAYER,			// There is one player, he can drive, but when 2nd player joins, we switch to PREPARING: state
	PREPARING,			// Vehicles are being spawned and controls are locked
	COUNTDOWN,			// No new vehicles are being spawned
	RACING,				// Race goes on
	FINISH_SCREEN		// Showing the screen with race results
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
	
	// States of the race
	[RplProp()]
	protected ERaceState m_eRaceState = -1;
	protected UDR_RaceStateBase m_RaceState;
	protected ref UDR_RaceStateNoPlayers		m_StateNoPlayers;
	protected ref UDR_RaceStateOnePlayer		m_StateOnePlayer;
	protected ref UDR_RaceStatePreparing		m_StatePreparing;
	protected ref UDR_RaceStateCountdown		m_StateCountdown;
	protected ref UDR_RaceStateRacing			m_StateRacing;
	protected ref UDR_RaceStateFinishScreen		m_StateFinishScreen;
	
	
	// IDs of players which are assigned to the race
	protected ref array<UDR_PlayerNetworkComponent> m_aCurrentRacers = {};	// Players who are currently racing
	protected ref array<UDR_PlayerNetworkComponent> m_aNextRacers = {};		// Players who want to race at the next race
	protected ref array<UDR_PlayerNetworkComponent> m_aSpectators = {};		// Players currently spectating
	
	protected int m_iNextFreeSpawnPointId = 0;
	
	// Map which maintains UDR_PlayerNetworkEntity per each player
	protected ref map<int, UDR_PlayerNetworkEntity> m_mPlayerNetworkSyncEntities = new map<int, UDR_PlayerNetworkEntity>();
	
	//-------------------------------------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		if (!GetGame().InPlayMode())
			return;
		
		if (m_RplComponent.IsMaster())
		{
			//--------------------------------------------------------
			// Everything below is only for server
			
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
			
			// Initialize the race states
			m_StateNoPlayers 		= new UDR_RaceStateNoPlayers(this);
			m_StateOnePlayer		= new UDR_RaceStateOnePlayer(this);
			m_StatePreparing		= new UDR_RaceStatePreparing(this);
			m_StateCountdown		= new UDR_RaceStateCountdown(this);
			m_StateRacing			= new UDR_RaceStateRacing(this);
			m_StateFinishScreen		= new UDR_RaceStateFinishScreen(this);
			SwitchToRaceState(ERaceState.NO_PLAYERS);
		}
	}
		
	//-------------------------------------------------------------------------------------------------------------------------------
	override void OnPlayerRegistered(int playerId)
	{
		super.OnPlayerRegistered(playerId);
		
		if (!m_RplComponent.IsMaster())
			return;
		
		//--------------------------------------------------------
		// Everything below is only for server
		
		// Create network sync entity
		Resource res = Resource.Load("{788E4A6E27537FDD}Prefabs/GameMode/PlayerNetworkSyncEntity.et");
		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.Parent = this;
		UDR_PlayerNetworkEntity networkSyncEntity = UDR_PlayerNetworkEntity.Cast(GetGame().SpawnEntityPrefab(res, params: spawnParams));
		networkSyncEntity.Init(playerId);
		RegisterPlayerNetrowkSyncEntity(networkSyncEntity);
		
		// Init player network component
		UDR_PlayerNetworkComponent playerComp = UDR_PlayerNetworkComponent.GetForPlayerId(playerId);
		
		// Finish registration later
		GetGame().GetCallqueue().CallLater(OnAfterPlayerRegistered, 0, false, playerId); // No idea why but spawning doesn't work instantly, must be called later
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void OnAfterPlayerRegistered(int playerId)
	{
		UDR_PlayerNetworkComponent playerComp = UDR_PlayerNetworkComponent.GetForPlayerId(playerId);
		
		// The function is called delayed, player might have disconnected, better check that!
		if (!playerComp)
			return;
		
		m_RaceState.OnPlayerConnected(playerComp);		
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	override void OnPlayerDisconnected(int playerId)
	{
		super.OnPlayerDisconnected(playerId);
		
		if (!m_RplComponent.IsMaster())
			return;
		
		//--------------------------------------------------------
		// Everything below is only for server
		
		UDR_PlayerNetworkComponent playerComp = UDR_PlayerNetworkComponent.GetForPlayerId(playerId);
		if (playerComp)
			m_RaceState.OnPlayerDisconnected(playerComp);
		
		// Delete the network sync entity of this player
		UDR_PlayerNetworkEntity networkSyncEnt;
		if (m_mPlayerNetworkSyncEntities.Find(playerId, networkSyncEnt))
		{
			SCR_Global.DeleteEntityAndChildren(networkSyncEnt);
		}
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void RegisterPlayerNetrowkSyncEntity(notnull UDR_PlayerNetworkEntity networkEntity)
	{
		if (m_mPlayerNetworkSyncEntities.Contains(networkEntity.m_iPlayerId))
			return;
		
		m_mPlayerNetworkSyncEntities.Insert(networkEntity.m_iPlayerId, networkEntity);
		
		// Register the network entity at player controller too
		// This is only going to happen for the client which owns this player controller
		UDR_PlayerNetworkComponent playerComp = UDR_PlayerNetworkComponent.GetForPlayerId(networkEntity.m_iPlayerId);
		if (playerComp)
		{
			playerComp.Init(networkEntity.m_iPlayerId, networkEntity);
		}
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void UnregisterPlayerNetrowkSyncEntity(notnull UDR_PlayerNetworkEntity ent)
	{
		m_mPlayerNetworkSyncEntities.Remove(ent.m_iPlayerId);
	}
	
	
	
	//-------------------------------------------------------------------------------------------------------------------------------
	// Vehicle spawning
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void SpawnVehicleAtSpawnPoint(notnull UDR_PlayerNetworkComponent playerComp)
	{
		vector transform[4];
		int spawnPointId = FindNextFreeSpawnPoint();
		m_VehiclePositioning.GetPosition(spawnPointId, transform); // Get position from vehicle positioning entity
		SpawnVehicle(playerComp, transform);
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void SpawnVehicleAtWaypoint(notnull UDR_PlayerNetworkComponent playerComp, int waypointId)
	{
		// todo
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void SpawnVehicle(notnull UDR_PlayerNetworkComponent playerComp, vector transform[4])
	{
		SCR_PlayerController pc = SCR_PlayerController.Cast(playerComp.GetOwner());
	
		// If there is a previous vehicle or character controlled by this player, delete them
		DespawnVehicle(playerComp);
			
		// Spawn driver character
		EntitySpawnParams p = new EntitySpawnParams();
		p.Transform[3] = m_VehiclePositioning.GetOrigin();
		Resource playerRes = Resource.Load("{A8BE87DC32CFF3C5}Prefabs/Characters/DriverCharacter.et");
		IEntity controlledEntity = GetGame().SpawnEntityPrefab(playerRes, params: p);
		
		pc.SetPossessedEntity(controlledEntity);
		
		// Spawn vehicle
		// List of UDR custom vehicles
		array<ResourceName> vehiclePrefabs = {
			"{1A20D130A03F9CF1}Prefabs/Vehicles/Wheeled/UAZ469/UAZ469_Armed.et",
			"{21C45FA677BCDBDA}Prefabs/Vehicles/Wheeled/M998/M998_Armed.et"
		};

		
		// Resolve spawn position
		EntitySpawnParams spawnParams = new EntitySpawnParams();
		for (int i = 0; i < 4; i++)
			spawnParams.Transform[i] = transform[i];
		
		Resource vehicleRes = Resource.Load(vehiclePrefabs[0]);
		IEntity newVehicleEntity = GetGame().SpawnEntityPrefab(vehicleRes, params: spawnParams);
		
		// Register the vehicle to race track logic
		m_RaceTrackLogic.RegisterRacer(newVehicleEntity);
		
		// Register to destroyed event
		EventHandlerManagerComponent ev = EventHandlerManagerComponent.Cast(newVehicleEntity.FindComponent(EventHandlerManagerComponent));
        ev.RegisterScriptHandler("OnDestroyed", newVehicleEntity, OnVehicleDestroyed);
		
		// Initialize vehicleComp
		Vehicle vehicle = Vehicle.Cast(newVehicleEntity);
		UDR_VehicleNetworkComponent vehNetComp = UDR_VehicleNetworkComponent.Cast(vehicle.FindComponent(UDR_VehicleNetworkComponent));
		vehNetComp.Init(playerComp.GetPlayerId());

		// Start engine
		CarControllerComponent carController = CarControllerComponent.Cast(vehicle.FindComponent(CarControllerComponent));
		carController.StartEngine();
		
		// Move player in pilot and start the car
		playerComp.MoveInVehicle(vehicle);
		
		SCR_DamageManagerComponent damageManager = SCR_DamageManagerComponent.Cast(controlledEntity.FindComponent(SCR_DamageManagerComponent));
		if (damageManager)
			damageManager.EnableDamageHandling(false);
				
		
		// Assign vehicle to player
		playerComp.m_AssignedVehicle = newVehicleEntity;
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	int FindNextFreeSpawnPoint()
	{
		int id = m_iNextFreeSpawnPointId;
		m_iNextFreeSpawnPointId++;
		return id;
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void DespawnVehicle(notnull UDR_PlayerNetworkComponent playerComp)
	{
		SCR_PlayerController pc = SCR_PlayerController.Cast(playerComp.GetOwner());
		if (pc.GetControlledEntity())
		{
			pc.SetPossessedEntity(null);
			SCR_Global.DeleteEntityAndChildren(pc.GetControlledEntity());
		}
		if (playerComp.m_AssignedVehicle)
		{
			SCR_Global.DeleteEntityAndChildren(playerComp.m_AssignedVehicle);
		}
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void DespawnAllVehicles()
	{
		foreach (UDR_PlayerNetworkComponent playerComp : GetAllPlayers())
		{
			DespawnVehicle(playerComp);
		}
	}
	
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void OnVehicleDestroyed(IEntity vehEntity)
	{
		m_RaceTrackLogic.UnregisterRacer(vehEntity);
		
		int playerID = UDR_VehicleNetworkComponent.Cast(vehEntity.FindComponent(UDR_VehicleNetworkComponent)).GetPlayerId();
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
		
		return vehNetworkComp.GetPlayerId();
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	UDR_PlayerNetworkEntity GetPlayerNetworkSyncEntity(int playerId)
	{
		return m_mPlayerNetworkSyncEntities.Get(playerId);
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	float m_fUpdateRaceTrackLogicTimer = RACE_TRACK_LOGIC_UPDATE_INTERVAL;
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);
		
		
		// Disable car inputs during countdown
		if (m_eRaceState == ERaceState.COUNTDOWN || m_eRaceState == ERaceState.PREPARING)
		{
			auto im = GetGame().GetInputManager();
			im.ActivateContext("UDR_CountdownContext", 50);
			
			/*
			// Doesn't work when run on client unfortunately
			im.SetActionValue("CarThrust", 0);
			im.SetActionValue("CarBrake", 0);
			im.SetActionValue("CarHandBrake", 1.0);
			im.SetActionValue("VehicleFire", 0);
			*/
		}
		
		
		
		if (m_RplComponent.IsMaster())
		{
			//--------------------------------------------------------
			// Everything below is only for server
			
			// Update the race track logic based on timer (no need to update too often)
			m_fUpdateRaceTrackLogicTimer += timeSlice;
			if (m_fUpdateRaceTrackLogicTimer >= RACE_TRACK_LOGIC_UPDATE_INTERVAL)
			{
				UpdateRaceTrackLogic(timeSlice);			
				m_fUpdateRaceTrackLogicTimer -= RACE_TRACK_LOGIC_UPDATE_INTERVAL;
			}
			
			UpdateRaceState(timeSlice);
		}
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void UpdateRaceTrackLogic(float timeSlice)
	{
		m_RaceTrackLogic.UpdateAllRacers();
			
		// Update player component of each player
		PlayerManager playerMgr = GetGame().GetPlayerManager();
		array<int> players = {};
		playerMgr.GetPlayers(players);
		
		array<UDR_PlayerNetworkComponent> playerComponentsSorted = {};
		foreach (int playerId : players)
		{
			PlayerController playerController = playerMgr.GetPlayerController(playerId);
			if (!playerController)
				continue;
			UDR_PlayerNetworkComponent playerComp = UDR_PlayerNetworkComponent.Cast(playerController.FindComponent(UDR_PlayerNetworkComponent));
			if (!playerComp)
				continue;
			
			UDR_PlayerNetworkEntity networkEnt = playerComp.m_NetworkEntity;
			
			IEntity assignedVehicle = playerComp.m_AssignedVehicle;
			if (!assignedVehicle)
				continue;
			
			// Get race track data of the player
			float totalProgress;
			int lapCount;
			int nextWaypoint;
			if (!m_RaceTrackLogic.GetRacerData(assignedVehicle, totalProgress, lapCount, nextWaypoint))
				continue;
			
			networkEnt.m_iLapCount = lapCount;
			playerComp.m_iNextWaypoint = nextWaypoint;
			
			playerComp.m_fTotalProgress = totalProgress;
			
			playerComponentsSorted.Insert(playerComp);
		}
		
		// Sort player components by total track progress, to find who is first, second, etc
		SCR_Sorting<UDR_PlayerNetworkComponent, UDR_PlayerNetworkComponent_CompareTotalProgress>.HeapSort(playerComponentsSorted, true);
		
		foreach (int i, UDR_PlayerNetworkComponent playerComp : playerComponentsSorted)
		{
			UDR_PlayerNetworkEntity networkSync = playerComp.m_NetworkEntity;
			
			networkSync.m_iPositionInRace = i;
			networkSync.BumpReplication();
		}
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void UpdateRaceState(float timeSlice)
	{
		RemoveNullsFromArray(m_aCurrentRacers);
		RemoveNullsFromArray(m_aNextRacers);
		RemoveNullsFromArray(m_aSpectators);
		
		if (m_RaceState)
		{
			ERaceState newRaceState = -1;
			bool switchToNewState = m_RaceState.OnUpdate(timeSlice, newRaceState);
			if (switchToNewState && newRaceState != -1)
				SwitchToRaceState(newRaceState);
		}
	}
	protected static void RemoveNullsFromArray(array<UDR_PlayerNetworkComponent> a)
	{
		for (int i = a.Count() - 1; i >= 0; i--)
		{
			if (!a[i])
				a.Remove(i);
		}
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void AssignToNextRace(notnull UDR_PlayerNetworkComponent playerComp)
	{
		_print(string.Format("AssignToNextRace: %1", playerComp.GetPlayerName()));
		
		if (!m_aNextRacers.Contains(playerComp))
			m_aNextRacers.Insert(playerComp);
		
		playerComp.m_NetworkEntity.m_bAssignedForRace = true;
		playerComp.m_NetworkEntity.BumpReplication();
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void UnassignFromNextRace(notnull UDR_PlayerNetworkComponent playerComp)
	{
		_print(string.Format("UnassignFromNextRace: %1", playerComp.GetPlayerName()));
		
		m_aNextRacers.RemoveItem(playerComp);
		
		playerComp.m_NetworkEntity.m_bAssignedForRace = false;
		playerComp.m_NetworkEntity.BumpReplication();
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void AssignToSpectators(notnull UDR_PlayerNetworkComponent playerComp)
	{
		_print(string.Format("AssignToSpectators: %1", playerComp.GetPlayerName()));
		
		if (!m_aSpectators.Contains(playerComp))
			m_aSpectators.Insert(playerComp);
		
		playerComp.m_NetworkEntity.m_bSpectating = true;
		playerComp.m_NetworkEntity.BumpReplication();
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void UnassignFromSpectators(notnull UDR_PlayerNetworkComponent playerComp)
	{
		_print(string.Format("UnassignFromSpectators: %1", playerComp.GetPlayerName()));
		
		m_aSpectators.RemoveItem(playerComp);
		playerComp.m_NetworkEntity.m_bSpectating = false;
		playerComp.m_NetworkEntity.BumpReplication();
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	// Getters for arrays
	
	array<UDR_PlayerNetworkComponent> GetCurrentRacers()
	{
		RemoveNullsFromArray(m_aCurrentRacers);
		array<UDR_PlayerNetworkComponent> a = {};
		a.Copy(m_aCurrentRacers);
		return a;
	}
	
	array<UDR_PlayerNetworkComponent> GetNextRacers()
	{
		RemoveNullsFromArray(m_aNextRacers);
		array<UDR_PlayerNetworkComponent> a = {};
		a.Copy(m_aNextRacers);
		return a;
	}
	
	array<UDR_PlayerNetworkComponent> GetSpectators()
	{
		RemoveNullsFromArray(m_aSpectators);
		array<UDR_PlayerNetworkComponent> a = {};
		a.Copy(m_aSpectators);
		return a;
	}
	
	// Returns all players, regardless of their state
	array<UDR_PlayerNetworkComponent> GetAllPlayers()
	{
		PlayerManager pm = GetGame().GetPlayerManager();
		array<int> playerIds = {};
		pm.GetPlayers(playerIds);
		
		array<UDR_PlayerNetworkComponent> a = {};
		foreach (int id : playerIds)
		{
			auto playerComp = UDR_PlayerNetworkComponent.GetForPlayerId(id);			
			a.Insert(playerComp);
		}
		
		return a;
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	string GetNotificationText()
	{	
		switch (m_eRaceState)
		{
			case ERaceState.ONE_PLAYER:
			{
				return "You are the only player. Drive around until more players join...";
			}
			
			case ERaceState.PREPARING:
			{
				return "Prepare for race! Wait for a few seconds...";
			}
			
			case ERaceState.RACING:
			case ERaceState.COUNTDOWN:
			case ERaceState.FINISH_SCREEN:
			{
				return string.Empty;
			}
		}
		
		return string.Empty;
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	protected void SwitchToRaceState(ERaceState newRaceState)
	{
		if (m_eRaceState == newRaceState)
			return;
		
		if (m_RaceState)
		{
			_print(string.Format("Leaving race state: %1", m_RaceState.Type()));
			m_RaceState.OnStateLeave();
		}
		
		switch (newRaceState)
		{
			case ERaceState.NO_PLAYERS:			m_RaceState = m_StateNoPlayers; break;
			case ERaceState.ONE_PLAYER:			m_RaceState = m_StateOnePlayer; break;
			case ERaceState.PREPARING:			m_RaceState = m_StatePreparing; break;
			case ERaceState.COUNTDOWN:			m_RaceState = m_StateCountdown; break;
			case ERaceState.RACING:				m_RaceState = m_StateRacing; break;
			case ERaceState.FINISH_SCREEN:		m_RaceState = m_StateFinishScreen; break;
		}
		
		_print(string.Format("Entering race state: %1", m_RaceState.Type()));
		m_eRaceState = newRaceState;
		m_RaceState.OnStateEnter();
		Replication.BumpMe();
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void _print(string str, LogLevel logLevel = LogLevel.NORMAL)
	{
		Print(string.Format("UDR_GameMode: %1", str));
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	// Sends a UI sound event to all users
	void BroadcastUiSoundEvent(string eventName)
	{
		foreach (UDR_PlayerNetworkComponent playerComp : GetAllPlayers())
		{
			playerComp.Authority_SendUiSoundEvent(eventName);
		}
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	// Sends a UI sound event to all users
	void BroadcastNotification(string text, float lifeTime_ms)
	{
		foreach (UDR_PlayerNetworkComponent playerComp : GetAllPlayers())
		{
			playerComp.Authority_SendNotification(text, lifeTime_ms);
		}
	}
}