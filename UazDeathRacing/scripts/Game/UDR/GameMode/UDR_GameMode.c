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
	RESULTS				// Showing the screen with race results
}

class UDR_GameMode: SCR_BaseGameMode
{
	// Constants
	protected const float RACE_TRACK_LOGIC_UPDATE_INTERVAL = 0.25;
	protected const int REVENGE_DURATION_MS = 5000;
	
	[Attribute()]
	protected ref array<ref UDR_EntityLinkRaceTrackLogic> m_aRaceTracks;
	
	// States of the race
	[RplProp()]
	protected ERaceState m_eRaceState = -1;
	
	[RplProp()]
	protected int m_iCurrentRaceTrackId;
	protected UDR_RaceTrackLogic m_CurrentRaceTrack;
	
	protected UDR_RaceStateBase m_RaceState;
	protected ref UDR_RaceStateNoPlayers		m_StateNoPlayers;
	protected ref UDR_RaceStateOnePlayer		m_StateOnePlayer;
	protected ref UDR_RaceStatePreparing		m_StatePreparing;
	protected ref UDR_RaceStateCountdown		m_StateCountdown;
	protected ref UDR_RaceStateRacing			m_StateRacing;
	protected ref UDR_RaceStateResults			m_StateResults;
	
	// Map which maintains UDR_PlayerNetworkEntity per each player
	protected ref map<int, UDR_PlayerNetworkEntity> m_mPlayerNetworkSyncEntities = new map<int, UDR_PlayerNetworkEntity>();
	
	// Table with race results
	// On clients it's updated via RPC
	protected ref UDR_RaceResultsTable m_RaceResultsTable = new UDR_RaceResultsTable();
	
	// Local chat command handler component is registered here
	UDR_ChatCommandHandlerComponent m_LocalChatCommandHandler;
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void UDR_GameMode(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT | EntityEvent.FRAME | EntityEvent.DIAG);
	}
	
	
	//-------------------------------------------------------------------------------------------------------------------------------
	// PLAYER CONNECTION HANDLING
	
	static void _____PLAYER_CONNECTION_HANDLING();
	
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
		
		// Unassign spawn position
		GetVehiclePositioning().UnassignPlayer(playerId);
		
		// Notify the race state
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
	// PLAYER ASSIGNMENT TO GROUPS
	
	static void _____PLAYER_GROUP_ASSIGNMENT();
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void AssignToCurrentRace(notnull UDR_PlayerNetworkComponent playerComp, bool value)
	{
		_print(string.Format("AssignToCurrentRace: %1 %2", playerComp.GetPlayerName(), value));
		playerComp.m_NetworkEntity.m_bRacingNow = value;
		playerComp.m_NetworkEntity.BumpReplication();
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void AssignToNextRace(notnull UDR_PlayerNetworkComponent playerComp, bool value)
	{
		_print(string.Format("AssignToNextRace: %1 %2", playerComp.GetPlayerName(), value));
		playerComp.m_NetworkEntity.m_bAssignedForNextRace = value;
		playerComp.m_NetworkEntity.BumpReplication();
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void AssignToSpectators(notnull UDR_PlayerNetworkComponent playerComp, bool value)
	{
		_print(string.Format("AssignToSpectators: %1 %2", playerComp.GetPlayerName(), value));
		playerComp.m_NetworkEntity.m_bSpectating = value;
		playerComp.m_NetworkEntity.BumpReplication();
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void UnassignAllFromCurrentRace()
	{
		_print("UnassignAllFromCurrentRace");
		
		foreach (UDR_PlayerNetworkComponent playerComp : UDR_PlayerNetworkComponent.GetAll())
			AssignToCurrentRace(playerComp, false);
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void UnassignAllFromNextRace()
	{
		_print("UnassignAllFromNextRace");
		
		foreach (UDR_PlayerNetworkComponent playerComp : UDR_PlayerNetworkComponent.GetAll())
			AssignToNextRace(playerComp, false);
	}
	
	
	
	
	
	//-------------------------------------------------------------------------------------------------------------------------------
	// RESPAWNING
	
	static void _____RESPAWNING();
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void SpawnVehicleAtSpawnPoint(notnull UDR_PlayerNetworkComponent playerComp)
	{
		vector transform[4];
		int spawnPointId = FindAndAssignSpawnPosition(playerComp);
		GetVehiclePositioning().GetPositionTransform(spawnPointId, transform); // Get position from vehicle positioning entity
		SpawnVehicle(playerComp, transform, 0, 1); // On first spawn we always have 0 lap count and our first waypoint is wp 1.
		playerComp.Authority_HighlightWaypoint(m_iCurrentRaceTrackId, 1);
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void SpawnVehicleAtLastWaypoint(notnull UDR_PlayerNetworkComponent playerComp)
	{
		vector transform[4];
		UDR_Waypoint wp = m_CurrentRaceTrack.GetWaypoint(playerComp.m_iPrevWaypoint);
		wp.GetTransform(transform);
		SpawnVehicle(playerComp, transform, playerComp.m_NetworkEntity.m_iLapCount, playerComp.m_iNextWaypoint);
		playerComp.Authority_HighlightWaypoint(m_iCurrentRaceTrackId, playerComp.m_iNextWaypoint);
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void SpawnVehicle(notnull UDR_PlayerNetworkComponent playerComp, vector transform[4], int lapCount, int nextWaypoint)
	{
		_print(string.Format("SpawnVehicle: %1, %2", playerComp.GetPlayerId(), playerComp.GetPlayerName()));
		
		SCR_PlayerController pc = SCR_PlayerController.Cast(playerComp.GetOwner());
	
		// If there is a previous vehicle or character controlled by this player, delete them
		DespawnVehicle(playerComp);
			
		// Spawn driver character
		EntitySpawnParams p = new EntitySpawnParams();
		p.Transform[3] = GetVehiclePositioning().GetOrigin();
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
		
		// Register to destroyed event
		EventHandlerManagerComponent ev = EventHandlerManagerComponent.Cast(newVehicleEntity.FindComponent(EventHandlerManagerComponent));
        ev.RegisterScriptHandler("OnDestroyed", newVehicleEntity, Callback_OnVehicleDestroyed);

		// Initialize vehicleComp
		Vehicle vehicle = Vehicle.Cast(newVehicleEntity);
		UDR_VehicleNetworkComponent vehNetComp = UDR_VehicleNetworkComponent.Cast(vehicle.FindComponent(UDR_VehicleNetworkComponent));
		vehNetComp.Init(playerComp.GetPlayerId());
		
		// Start engine
		CarControllerComponent carController = CarControllerComponent.Cast(vehicle.FindComponent(CarControllerComponent));
		carController.StartEngine();
		
		// Move player in pilot
		playerComp.MoveInVehicle(vehicle);
		
		// Make player invincible
		SCR_DamageManagerComponent damageManager = SCR_DamageManagerComponent.Cast(controlledEntity.FindComponent(SCR_DamageManagerComponent));
		if (damageManager)
			damageManager.EnableDamageHandling(false);
				
		// Assign vehicle to player
		playerComp.m_AssignedVehicle = newVehicleEntity;
		RplComponent rpl = RplComponent.Cast(newVehicleEntity.FindComponent(RplComponent));
		playerComp.m_NetworkEntity.m_AssigedVehicleId = rpl.Id();
		playerComp.m_NetworkEntity.BumpReplication();
		
		// Register the vehicle to race track logic
		m_CurrentRaceTrack.RegisterRacer(newVehicleEntity, lapCount, nextWaypoint);
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void DespawnVehicle(notnull UDR_PlayerNetworkComponent playerComp)
	{
		_print(string.Format("DespawnVehicle: %1, %2", playerComp.GetPlayerId(), playerComp.GetPlayerName()));
		
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
		foreach (UDR_PlayerNetworkComponent playerComp : UDR_PlayerNetworkComponent.GetAll())
		{
			DespawnVehicle(playerComp);
		}
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	// Kills player and his vehicle if it's not destroyed yet
	void DestroyPlayerAndVehicle(notnull UDR_PlayerNetworkComponent playerComp)
	{
		PlayerController pc = PlayerController.Cast(playerComp.GetOwner());
		if (!pc)
			return;
		IEntity controlledEntity = pc.GetControlledEntity();
		SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(controlledEntity.FindComponent(SCR_CharacterControllerComponent));
		if (characterController)
			characterController.ForceDeath();
		
		if (playerComp.m_AssignedVehicle)
		{
			SCR_VehicleDamageManagerComponent vehicleDamageMgr = SCR_VehicleDamageManagerComponent.Cast(
				playerComp.m_AssignedVehicle.FindComponent(SCR_VehicleDamageManagerComponent));
			vehicleDamageMgr.Kill();
		}
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	int FindAndAssignSpawnPosition(notnull UDR_PlayerNetworkComponent playerComp)
	{
		int playerId = playerComp.GetPlayerId();
		
		// Check if a position has been assigned to this player already
		int previousAssignedPosition = GetVehiclePositioning().FindAssignedPosition(playerId);
		
		if (previousAssignedPosition != -1)
			return previousAssignedPosition;
		
		// If not, find a next free position
		int nextFreePosition = GetVehiclePositioning().FindNextFreePosition();
		
		if (nextFreePosition != -1)
		{
			GetVehiclePositioning().AssignPosition(nextFreePosition, playerId);
			return nextFreePosition;
		}
		else
		{
			// No more positions :( just select a random one, but don't assign it
			return GetVehiclePositioning().GetRandomPosition();
		}
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void EndRevenge(int playerId)
	{
		// Check if player has a working vehicle already - meaning he has already respawned
		UDR_PlayerNetworkComponent playerComp = UDR_PlayerNetworkComponent.GetForPlayerId(playerId);
		if (!playerComp)
			return;
		
		bool assignedVehicleAlive = false;
		IEntity assignedVehicle = playerComp.m_AssignedVehicle;
		if (assignedVehicle)
		{
			SCR_DamageManagerComponent vehicleDamageMgr = SCR_DamageManagerComponent.Cast(assignedVehicle.FindComponent(SCR_DamageManagerComponent));
			if (vehicleDamageMgr)
			{
				assignedVehicleAlive = vehicleDamageMgr.GetState() != EDamageState.DESTROYED;
			}
		}
		
		// Bail if we have a vehicle already
		if (assignedVehicleAlive)
			return;
		
		// If vehicle is not alive, kill player
		// The player character death events will handle the rest
		DestroyPlayerAndVehicle(playerComp);
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void RespawnPlayer(notnull UDR_PlayerNetworkComponent playerComp)
	{
		if (!playerComp.m_NetworkEntity.m_bSpectating)
		{
			SpawnVehicleAtLastWaypoint(playerComp);
		}
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	// EVENT HANDLERS
	
	static void _____EVENT_HANDLERS();
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void Callback_OnVehicleDestroyed(IEntity vehEntity)
	{
		/*
		m_CurrentRaceTrack.UnregisterRacer(vehEntity);
		
		int playerID = UDR_VehicleNetworkComponent.Cast(vehEntity.FindComponent(UDR_VehicleNetworkComponent)).GetPlayerId();
		if (!playerID) {
			Print("no player found attached to this vehicle");
			return;
		}

		GetGame().GetCallqueue().CallLater(ForceRespawnPlayer, 5000, false, playerID);
		*/
		
		UDR_VehicleNetworkComponent vehicleComp = UDR_VehicleNetworkComponent.Cast(vehEntity.FindComponent(UDR_VehicleNetworkComponent));
		if (!vehicleComp)
			return;
		
		int playerId = vehicleComp.GetPlayerId();
		
		UDR_PlayerNetworkComponent playerComp = UDR_PlayerNetworkComponent.GetForPlayerId(playerId);
		if (!playerComp)
			return;
		
		// Enable back damage for all occupants
		/*
		BaseCompartmentManagerComponent compartmentMgr = BaseCompartmentManagerComponent.Cast(vehEntity.FindComponent(BaseCompartmentManagerComponent));
		array<BaseCompartmentSlot> compartments = {};
		compartmentMgr.GetCompartments(compartments);
		foreach (BaseCompartmentSlot slot : compartments)
		{
			IEntity occupant = slot.GetOccupant();
			if (!occupant)
				continue;
			
			/*
			SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(
				occupant.FindComponent(SCR_CharacterControllerComponent));
			if (!characterController)
				continue;
			
			characterController.ForceDeath();
			
		}
		*/
		
		// Get current race progress
		// And save it to player component. We must restore it upon respawn.
		float totalProgress;
		if (!m_CurrentRaceTrack.GetRacerData(vehEntity, totalProgress, playerComp.m_NetworkEntity.m_iLapCount,
				playerComp.m_iNextWaypoint, playerComp.m_iPrevWaypoint))
		{
			return;
		}
			
		m_CurrentRaceTrack.UnregisterRacer(vehEntity);
		
		// Enable back damage handling for character
		PlayerController pc = PlayerController.Cast(playerComp.GetOwner());
		IEntity controlledEntity = pc.GetControlledEntity();
		if(controlledEntity)
		{
			SCR_DamageManagerComponent damageMgr = SCR_DamageManagerComponent.Cast(controlledEntity.FindComponent(SCR_DamageManagerComponent));
			if (damageMgr)
			{
				damageMgr.EnableDamageHandling(true);
			}
		}
		
		GetGame().GetCallqueue().CallLater(EndRevenge, REVENGE_DURATION_MS, false, playerId);
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	// Called from race track logic when a racer (vehicle) crosses finish line
	void Callback_OnFinishLineActivated(IEntity racer, bool lastLap)
	{
		UDR_VehicleNetworkComponent vehicleComp = UDR_VehicleNetworkComponent.Cast(racer.FindComponent(UDR_VehicleNetworkComponent));
		if (!vehicleComp)
			return;
		
		UDR_PlayerNetworkComponent playerComp = UDR_PlayerNetworkComponent.GetForPlayerId(vehicleComp.GetPlayerId());
		if (!playerComp)
			return;
		
		_print(string.Format("Player has crossed finished line: %1", playerComp.GetPlayerName()));
		
		if (lastLap)
		{
			m_RaceState.OnPlayerFinishedRace(playerComp);
		}
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	// Called from race track logic when a racer (vehicles) crosses any waypoint
	void Callback_OnWaypointActivated(IEntity racer, int wpId, int nextWpId)
	{
		UDR_VehicleNetworkComponent vehicleComp = UDR_VehicleNetworkComponent.Cast(racer.FindComponent(UDR_VehicleNetworkComponent));
		if (!vehicleComp)
			return;
		
		UDR_PlayerNetworkComponent playerComp = UDR_PlayerNetworkComponent.GetForPlayerId(vehicleComp.GetPlayerId());
		if (!playerComp)
			return;
		
		// Highlight next waypoint for client
		playerComp.Authority_HighlightWaypoint(m_iCurrentRaceTrackId, nextWpId);
		
		// Play UI sound event
		UDR_Waypoint wp = GetCurrentRaceTrack().GetWaypoint(wpId);
		string uiSoundEvent = wp.GetUiSoundEventName();
		if (!uiSoundEvent.IsEmpty())
			playerComp.Authority_SendUiSoundEvent(uiSoundEvent);
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	override void OnPlayerKilled(int playerId, IEntity player, IEntity killer)
	{
		super.OnPlayerKilled(playerId, player, killer);
		
		GetGame().GetCallqueue().CallLater(Callback_OnPlayerKilledDelayed, 0, false, playerId);
	}
	protected void Callback_OnPlayerKilledDelayed(int playerId)
	{
		UDR_PlayerNetworkComponent playerComp = UDR_PlayerNetworkComponent.GetForPlayerId(playerId);
		if (!playerComp)
			return;
		
		RespawnPlayer(playerComp);
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	override void OnPlayerRoleChange(int playerId, EPlayerRole roleFlags)
	{
		super.OnPlayerRoleChange(playerId, roleFlags);
		
		// When we gain admin rights, show a message to us locally
		if ((roleFlags & EPlayerRole.ADMINISTRATOR) || (roleFlags & EPlayerRole.SESSION_ADMINISTRATOR))
		{
			if (playerId == GetGame().GetPlayerController().GetPlayerId())
			{
				SCR_ChatPanelManager chatPanelMgr = SCR_ChatPanelManager.GetInstance();
				string msg = "Type /help to view available commands";
				chatPanelMgr.OnNewMessage(msg);
			}
		}
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	// GETTERS
	
	static void _____GETTERS();
	
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
	UDR_PlayerNetworkEntity GetLocalPlayerNetworkEntity()
	{
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return null;
		return GetPlayerNetworkSyncEntity(pc.GetPlayerId());
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	// Returns all player nerwork entities
	array<UDR_PlayerNetworkEntity> GetAllPlayerNetworkEntities()
	{		
		array<UDR_PlayerNetworkEntity> a = {};
		
		foreach (int id, UDR_PlayerNetworkEntity ent : m_mPlayerNetworkSyncEntities)
		{		
			a.Insert(ent);
		}
		
		return a;
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	array<UDR_PlayerNetworkComponent> GetCurrentRacers()
	{
		array<UDR_PlayerNetworkComponent> a = {};
		foreach (UDR_PlayerNetworkComponent playerComp : UDR_PlayerNetworkComponent.GetAll())
		{
			if (playerComp.m_NetworkEntity && playerComp.m_NetworkEntity.m_bRacingNow)
				a.Insert(playerComp);
		}
		return a;
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	array<UDR_PlayerNetworkComponent> GetNextRacers()
	{
		array<UDR_PlayerNetworkComponent> a = {};
		foreach (UDR_PlayerNetworkComponent playerComp : UDR_PlayerNetworkComponent.GetAll())
		{
			if (playerComp.m_NetworkEntity && playerComp.m_NetworkEntity.m_bAssignedForNextRace)
				a.Insert(playerComp);
		}
		return a;
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	array<UDR_PlayerNetworkComponent> GetSpectators()
	{
		array<UDR_PlayerNetworkComponent> a = {};
		foreach (UDR_PlayerNetworkComponent playerComp : UDR_PlayerNetworkComponent.GetAll())
		{
			if (playerComp.m_NetworkEntity && playerComp.m_NetworkEntity.m_bSpectating)
				a.Insert(playerComp);
		}
		return a;
	}
	
	
	//-------------------------------------------------------------------------------------------------------------------------------
	// Returns race state
	ERaceState GetRaceState()
	{
		return m_eRaceState;
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	UDR_RaceResultsTable GetRaceResults()
	{
		return m_RaceResultsTable;
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	// Fallback position for the camera when there is nothing to spectate
	IEntity GetFallbackSpectatorTarget()
	{
		return m_aRaceTracks[m_iCurrentRaceTrackId].value;
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	UDR_VehiclePositioning GetVehiclePositioning()
	{
		return m_CurrentRaceTrack.GetVehiclePositioning();
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	UDR_RaceTrackLogic GetCurrentRaceTrack()
	{
		// We don't return the race track object directly because on clients only ID is replicated
		return m_aRaceTracks[m_iCurrentRaceTrackId].value;
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	array<UDR_RaceTrackLogic> GetAllRaceTracks()
	{
		array<UDR_RaceTrackLogic> a = {};
		a.Resize(m_aRaceTracks.Count());
		foreach (int i, UDR_EntityLinkRaceTrackLogic link : m_aRaceTracks)
		{
			a[i] = link.value;
		}
		return a;
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	UDR_RaceTrackLogic GetRaceTrack(int id)
	{
		if (id < 0 || id >= m_aRaceTracks.Count())
			return null;
		return m_aRaceTracks[id].value;
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	// NOTIFICATIONS
	
	static void _____NOTIFICATIONS();
	
	//-------------------------------------------------------------------------------------------------------------------------------
	// Returns static notification text
	string GetNotificationText()
	{	
		switch (m_eRaceState)
		{
			case ERaceState.ONE_PLAYER:
			{
				UDR_PlayerNetworkComponent playerComp = UDR_PlayerNetworkComponent.GetLocal();
				if (!playerComp.m_NetworkEntity)
					return string.Empty;
				
				if (!playerComp.m_NetworkEntity.m_bSpectating)
					return "You are the only player. Wait until more players join...";
				else
					return string.Empty;
				
				break;
			}
			
			case ERaceState.PREPARING:
			{
				UDR_RaceTrackLogic currentTrack = GetCurrentRaceTrack();
				string strTrackName;
				if (currentTrack)
				{
					strTrackName = string.Format("Current race track:\n%1\n\n", currentTrack.GetRaceTrackName());
				}
				return strTrackName + "Prepare for race! Wait for a few seconds...";
			}
			
			case ERaceState.RACING:
			case ERaceState.COUNTDOWN:
			case ERaceState.RESULTS:
			{
				return string.Empty;
			}
		}
		
		return string.Empty;
	}
	
	
	//-------------------------------------------------------------------------------------------------------------------------------
	// ENTITY EVENTS
	
	static void _____ENTITY_EVENTS();
	
	//-------------------------------------------------------------------------------------------------------------------------------
	float m_fUpdateRaceTrackLogicTimer = RACE_TRACK_LOGIC_UPDATE_INTERVAL;
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);
		
		
		// Disable car inputs during countdown
		if (m_eRaceState == ERaceState.COUNTDOWN || m_eRaceState == ERaceState.PREPARING)
		{
			auto im = GetGame().GetInputManager();
			
			// This context has special priority value, to be higher than car controls, so car controls are blocked
			im.ActivateContext("UDR_CountdownContext", 50);
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
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		if (!GetGame().InPlayMode())
			return;
		
		foreach (UDR_EntityLinkRaceTrackLogic raceTrackLink : m_aRaceTracks)
		{
			if (raceTrackLink.Init() == null)
				continue;
			
			raceTrackLink.value.m_OnFinishLineActivated.Insert(Callback_OnFinishLineActivated);
			raceTrackLink.value.m_OnWaypointActivated.Insert(Callback_OnWaypointActivated);
		}
		
		if (m_RplComponent.IsMaster())
		{
			//--------------------------------------------------------
			// Everything below is only for server
			
			// Initialize the race states
			m_StateNoPlayers 		= new UDR_RaceStateNoPlayers(this);
			m_StateOnePlayer		= new UDR_RaceStateOnePlayer(this);
			m_StatePreparing		= new UDR_RaceStatePreparing(this);
			m_StateCountdown		= new UDR_RaceStateCountdown(this);
			m_StateRacing			= new UDR_RaceStateRacing(this);
			m_StateResults			= new UDR_RaceStateResults(this);
			SwitchToRaceState(ERaceState.NO_PLAYERS);
			
			SwitchToRaceTrack(0);
		}
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	override void EOnDiag(IEntity owner, float timeSlice)
	{
		if (DiagMenu.GetBool(SCR_DebugMenuID.UDR_SHOW_GAME_MODE_PANEL))
		{
			DbgUI.Begin("Game Mode");
			
			if (m_CurrentRaceTrack)
				DbgUI.Text(string.Format("Race Track: %1, %2", m_iCurrentRaceTrackId, m_CurrentRaceTrack.GetRaceTrackName()));
			
			DbgUI.Text(string.Format("Race State: %1 %2", typename.EnumToString(ERaceState, m_eRaceState), m_RaceState.Type()));
			
			DbgUI.Text("Spectators:");
			foreach (UDR_PlayerNetworkComponent player : GetSpectators())
				DbgUI.Text(string.Format(" [S] %1", player.GetPlayerName()));
			
			DbgUI.Text("Current Race:");
			foreach (UDR_PlayerNetworkComponent player : GetCurrentRacers())
				DbgUI.Text(string.Format(" [R] %1", player.GetPlayerName()));
			
			DbgUI.Text("Next Race:");
			foreach (UDR_PlayerNetworkComponent player : GetNextRacers())
				DbgUI.Text(string.Format(" [N] %1", player.GetPlayerName()));
			
			DbgUI.Text("Spawn Position assignments:");
			int spawnPosCount = GetVehiclePositioning().GetPositionCount();
			string spawnPosStr;
			for (int i = 0; i < spawnPosCount; i++)
			{
				spawnPosStr = spawnPosStr + string.Format("%1 ", GetVehiclePositioning().GetPlayerAssignedToPosition(i));
			}
			DbgUI.Text(spawnPosStr);
			
			DbgUI.Text("Race Results Table:");
			foreach (UDR_RaceResultsEntry entry : m_RaceResultsTable.GetEntries())
			{
				DbgUI.Text(string.Format(" - %1 %2 %3", entry.m_iPlayerId, entry.m_sPlayerName, entry.m_iTotalTime_ms));
			}
			
			if (DbgUI.Button("State: Results"))
			{
				// For testing, add every player to the results table
				foreach (UDR_PlayerNetworkComponent playerComp : UDR_PlayerNetworkComponent.GetAll())
				{
					m_RaceResultsTable.Add(playerComp.GetPlayerId(), playerComp.GetPlayerName(), Math.RandomFloat(0, 1000*60*3));
				}
				SwitchToRaceState(ERaceState.RESULTS);
			}
			
			DbgUI.End();
		}
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	override void _WB_AfterWorldUpdate(float timeSlice)
	{
		foreach (auto raceTrackLink : m_aRaceTracks)
		{
			if (raceTrackLink)
				raceTrackLink.Draw(this);
		}
	}
	
	
	//-------------------------------------------------------------------------------------------------------------------------------
	// RACE STATE
	
	static void _____RACE_STATE_AND_LOGIC();
	
	//-------------------------------------------------------------------------------------------------------------------------------
	protected void SwitchToRaceTrack(int trackId)
	{
		if (trackId < 0 || trackId >= m_aRaceTracks.Count())
			return;
		
		m_CurrentRaceTrack = m_aRaceTracks[trackId].value;
		m_iCurrentRaceTrackId = trackId;
		Replication.BumpMe();
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void SwitchToNextRaceTrack()
	{
		int newId = (m_iCurrentRaceTrackId + 1) % m_aRaceTracks.Count();
		SwitchToRaceTrack(newId);
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
			case ERaceState.RESULTS:			m_RaceState = m_StateResults; break;
		}
		
		_print(string.Format("Entering race state: %1", m_RaceState.Type()));
		m_eRaceState = newRaceState;
		m_RaceState.OnStateEnter();
		Replication.BumpMe();
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void UpdateRaceState(float timeSlice)
	{
		if (m_RaceState)
		{
			ERaceState newRaceState = -1;
			bool switchToNewState = m_RaceState.OnUpdate(timeSlice, newRaceState);
			if (switchToNewState && newRaceState != -1)
				SwitchToRaceState(newRaceState);
		}
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void UpdateRaceTrackLogic(float timeSlice)
	{
		m_CurrentRaceTrack.UpdateAllRacers();
			
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
			
			IEntity assignedVehicle = playerComp.m_AssignedVehicle;
			if (!assignedVehicle)
				continue;
			
			// Get race track data of the player
			float totalProgress;
			int lapCount;
			int nextWaypoint;
			int prevWaypoint;
			if (!m_CurrentRaceTrack.GetRacerData(assignedVehicle, totalProgress, lapCount, nextWaypoint, prevWaypoint))
				continue;
			
			playerComp.m_iNextWaypoint = nextWaypoint;
			playerComp.m_iPrevWaypoint = prevWaypoint;
			playerComp.m_fTotalProgress = totalProgress;
			
			playerComp.m_NetworkEntity.m_iLapCount = lapCount;
			playerComp.m_NetworkEntity.BumpReplication();
			
			playerComponentsSorted.Insert(playerComp);
		}
		
		// Sort player components by total track progress, to find who is first, second, etc
		SCR_Sorting<UDR_PlayerNetworkComponent, UDR_PlayerNetworkComponent_CompareTotalProgress>.HeapSort(playerComponentsSorted, true);
		
		int nRacersFinishedRace = 0;	// Also count how many players have finished the race
		foreach (UDR_PlayerNetworkComponent playerComp : GetCurrentRacers())
			nRacersFinishedRace += playerComp.m_NetworkEntity.m_bFinishedRace;
		
		foreach (int i, UDR_PlayerNetworkComponent playerComp : playerComponentsSorted)
		{
			playerComp.m_NetworkEntity.m_iPositionInRace = i + nRacersFinishedRace;
			playerComp.m_NetworkEntity.BumpReplication();
		}
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	int AddToRaceResults(UDR_PlayerNetworkComponent playerComp)
	{
		int id = m_RaceResultsTable.GetCount();
		
		float timeSinceRaceStart_ms = m_CurrentRaceTrack.GetTimeSinceRaceStartMs();
		m_RaceResultsTable.Add(playerComp.GetPlayerId(), playerComp.GetPlayerName(), timeSinceRaceStart_ms);
		
		return id;
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void ClearRaceResults()
	{
		m_RaceResultsTable.Clear();
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void ResetRaceTimer()
	{
		m_CurrentRaceTrack.ResetRaceTimer();
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	// NETWORKING WITH CLIENT
	
	static void _____NETWORKING_WITH_CLIENT();
	
	//-------------------------------------------------------------------------------------------------------------------------------
	// Triggered by "Join Spectators" button
	void Ask_JoinSpectators(int playerId)
	{
		UDR_PlayerNetworkComponent playerComp = UDR_PlayerNetworkComponent.GetForPlayerId(playerId);
		
		if (!playerComp)
			return;
		
		// Player is spectator already
		if (playerComp.m_NetworkEntity.m_bSpectating)
			return;
		
		AssignToSpectators(playerComp, true);
		DespawnVehicle(playerComp);
		
		AssignToNextRace(playerComp, false);
		AssignToCurrentRace(playerComp, false);
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	// Triggered by "Join Race" button
	void Ask_JoinRace(int playerId)
	{
		UDR_PlayerNetworkComponent playerComp = UDR_PlayerNetworkComponent.GetForPlayerId(playerId);
		
		if (!playerComp)
			return;
		
		if (playerComp.m_NetworkEntity.m_bRacingNow || playerComp.m_NetworkEntity.m_bAssignedForNextRace)
			return;

		m_RaceState.OnPlayerRequestJoinRace(playerComp);
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	// Triggered by respawn action/button
	void Ask_Respawn(int playerId)
	{
		UDR_PlayerNetworkComponent playerComp = UDR_PlayerNetworkComponent.GetForPlayerId(playerId);
		if (!playerComp)
			return;
		
		_print(string.Format("Player requested respawn: %1 %2", playerComp.GetPlayerId(), playerComp.GetPlayerName()));
		m_RaceState.OnPlayerRequestRespawn(playerComp);
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	// Admin request to end the race
	void Ask_AdminEndRace()
	{
		RplComponent rpl = RplComponent.Cast(FindComponent(RplComponent));
		if (!rpl.IsMaster())
			return;
		
		SwitchToRaceState(ERaceState.RESULTS);
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void Ask_AdminSwitchRaceTrack(int trackId)
	{
		RplComponent rpl = RplComponent.Cast(FindComponent(RplComponent));
		if (!rpl.IsMaster())
			return;
		
		if (trackId < 0 || trackId >= m_aRaceTracks.Count())
			return;
		
		if (m_eRaceState == ERaceState.PREPARING)
			SwitchToRaceState(ERaceState.RESULTS);
		SwitchToRaceTrack(trackId);
		SwitchToRaceState(ERaceState.PREPARING);
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void BroadcastRaceResultsTable()
	{
		m_RaceResultsTable.InstantPack();
		string raceResultsJson = m_RaceResultsTable.AsString();
		//RpcDo_ShowResultTable(raceResultsJson); // No need to call it for ourselves, we have it already
		Rpc(RpcDo_UpdateRaceResultsTable, raceResultsJson);
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_UpdateRaceResultsTable(string raceResultsJson)
	{
		UDR_RaceResultsTable resultTable = new UDR_RaceResultsTable();
		resultTable.ExpandFromRAW(raceResultsJson);
		m_RaceResultsTable = resultTable; // Erase previous results
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	// Sends a UI sound event to all users
	void BroadcastUiSoundEvent(string eventName)
	{
		foreach (UDR_PlayerNetworkComponent playerComp : UDR_PlayerNetworkComponent.GetAll())
		{
			playerComp.Authority_SendUiSoundEvent(eventName);
		}
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	// Sends a notification to all players
	void BroadcastNotification(string text, float lifeTime_ms)
	{
		foreach (UDR_PlayerNetworkComponent playerComp : UDR_PlayerNetworkComponent.GetAll())
		{
			playerComp.Authority_SendNotification(text, lifeTime_ms);
		}
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	// OTHER
	
	static void _____OTHER();
	
	
	protected static void RemoveNullsFromArray(array<UDR_PlayerNetworkComponent> a)
	{
		for (int i = a.Count() - 1; i >= 0; i--)
		{
			if (!a[i])
				a.Remove(i);
		}
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void _print(string str, LogLevel logLevel = LogLevel.NORMAL)
	{
		Print(string.Format("UDR_GameMode: %1", str));
	}
}