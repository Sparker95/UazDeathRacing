[ComponentEditorProps(category: "UDR", description: "Component to be attached to PlayerController prefab. Used for game mode logic and network.")]
class UDR_PlayerNetworkComponentClass : ScriptComponentClass
{
}

class UDR_PlayerNetworkComponent : ScriptComponent
{
	UDR_PlayerNetworkEntity m_NetworkEntity;
	
	protected int m_iPlayerId;
	protected string m_sPlayerName;
	
	// Game mode logic
	IEntity m_AssignedVehicle;
	float m_fTotalProgress;	// Our total distance travelled, including previous laps
	int m_iNextWaypoint;
	int m_iPrevWaypoint;

  	// Used to avoid double death when runned over after vehicle was destroyed
	bool m_bHasDied = false;
  
	// For moving into vehicle
	protected bool m_bMoveInVehicleRequest = false;
	protected RplId m_MoveInVehicleRplId;
	
	// Array of notifications
	protected ref array<ref UDR_Notification> m_aNotifications = {};
		
	
	//-----------------------------------------------------------------------
	void BumpReplication()
	{
		Replication.BumpMe();
	}
	
	//-----------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		//owner.SetFlags(EntityFlags.ACTIVE, true);
		SetEventMask(owner, EntityEvent.FRAME);
		GetGame().GetInputManager().AddActionListener("UDR_Respawn", EActionTrigger.DOWN, OnRespawnAction);
	}
	
	//-----------------------------------------------------------------------
	void Init(int playerId, UDR_PlayerNetworkEntity networkSync)
	{
		m_iPlayerId = playerId;
		m_NetworkEntity = networkSync;
		m_sPlayerName = GetGame().GetPlayerManager().GetPlayerName(playerId);
	}
	
	//-----------------------------------------------------------------------
	int GetPlayerId() { return m_iPlayerId; }
	
	//-----------------------------------------------------------------------
	string GetPlayerName() { return m_sPlayerName; }
  
	//-----------------------------------------------------------------------
	// Playing sounds
	
	//-----------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_UiSoundEvent(string soundName)
	{
		SCR_UISoundEntity.SoundEvent(soundName);
	}
	
	//-----------------------------------------------------------------------
	void Authority_SendUiSoundEvent(string soundName)
	{
		Rpc(RpcDo_UiSoundEvent, soundName);
	}
	
	//-----------------------------------------------------------------------
	// Sending notifications
	
	//-----------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_AddNotification(string text, float lifeTime_ms)
	{
		float worldTime = GetGame().GetWorld().GetWorldTime();
		m_aNotifications.Insert(new UDR_Notification(text, lifeTime_ms));
	}
	
	//-----------------------------------------------------------------------
	void Authority_SendNotification(string text, float lifeTime_ms)
	{
		Rpc(RpcDo_AddNotification, text, lifeTime_ms);
	}
	
	//-----------------------------------------------------------------------
	array<ref UDR_Notification> GetNotifications()
	{
		return m_aNotifications;
	}
	
	//-----------------------------------------------------------------------
	// Moving in vehicle... is a whole serious process
	
	//-----------------------------------------------------------------------
	// Must be called by server
	void MoveInVehicle(Vehicle vehicle)
	{
		RplComponent rpl = RplComponent.Cast(vehicle.FindComponent(RplComponent));
		Rpc(RpcDo_MoveInVehicle, rpl.Id());
	}
	
	//-----------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_MoveInVehicle(RplId vehId)
	{
		m_bMoveInVehicleRequest = true;
		m_MoveInVehicleRplId = vehId;
	}
	
	//-----------------------------------------------------------------------
	void TryMoveInVehicle()
	{
		// We must wait until the vehicle entity is streamed to us, then move into it
		RplComponent rpl = RplComponent.Cast(Replication.FindItem(m_MoveInVehicleRplId));
		if (!rpl)
			return;
		
		Vehicle vehicle = Vehicle.Cast(rpl.GetEntity());
		if (!vehicle)
			return;
		
		PlayerController pc = PlayerController.Cast(GetOwner());
		IEntity controlledEntity = pc.GetControlledEntity();
		
		SCR_CompartmentAccessComponent compartmentAccessComponent = SCR_CompartmentAccessComponent.Cast(controlledEntity.FindComponent(SCR_CompartmentAccessComponent));
		compartmentAccessComponent.MoveInVehicle(vehicle, ECompartmentType.Pilot);
		
		m_bMoveInVehicleRequest = false;
	}
	
	
	//-------------------------------------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_JoinSpectators()
	{
		UDR_GameMode gm = GetGame().GetUdrGameMode();
		gm.Ask_JoinSpectators(m_iPlayerId);
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	// Called from client's UI when he wants to join spectators
	void Client_RequestJoinSpectators()
	{
		Rpc(RpcAsk_JoinSpectators);
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_JoinRace()
	{
		UDR_GameMode gm = GetGame().GetUdrGameMode();
		gm.Ask_JoinRace(m_iPlayerId);
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	// Called from client's UI when he wants to join race
	void Client_RequestJoinRace()
	{
		Rpc(RpcAsk_JoinRace);
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_Respawn()
	{
		GetGame().GetUdrGameMode().Ask_Respawn(m_iPlayerId);
	}
	
	//-----------------------------------------------------------------------
	// Called by the keyboard button action
	protected float m_fRespawnActionCooldownEnd_ms = 0;
	protected void OnRespawnAction()
	{
		float time_ms = GetGame().GetWorld().GetWorldTime();
		if (time_ms < m_fRespawnActionCooldownEnd_ms)
			return;
		
		Rpc(RpcAsk_Respawn);
		
		m_fRespawnActionCooldownEnd_ms = time_ms + 1000;
	}
	
	//-----------------------------------------------------------------------
	// Misc functions
	
	static UDR_PlayerNetworkComponent GetForPlayerId(int playerId)
	{
		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!pc)
			return null;
		UDR_PlayerNetworkComponent playerComp = UDR_PlayerNetworkComponent.Cast(pc.FindComponent(UDR_PlayerNetworkComponent));
		return playerComp;
	}
	
	// Returns the local player network component
	static UDR_PlayerNetworkComponent GetLocal()
	{
		PlayerController pc = GetGame().GetPlayerController();
		return UDR_PlayerNetworkComponent.Cast(pc.FindComponent(UDR_PlayerNetworkComponent));
	}
	
	// Returns component for all players, regardless of their state
	static array<UDR_PlayerNetworkComponent> GetAll()
	{
		PlayerManager pm = GetGame().GetPlayerManager();
		array<UDR_PlayerNetworkComponent> arrayOut = {};
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
			arrayOut.Insert(playerComp);
		}
		return arrayOut;
	}
	
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (m_bMoveInVehicleRequest)
		{
			TryMoveInVehicle();
		}
		
		// Delete old notifications
		float worldTime = GetGame().GetWorld().GetWorldTime();
		for (int i = m_aNotifications.Count() - 1; i >= 0; i--)
		{
			UDR_Notification n = m_aNotifications[i];
			if (worldTime > n.m_fTimeEnd)
				m_aNotifications.Remove(i);
		}
	}
}

// Used for sorting
class UDR_PlayerNetworkComponent_CompareTotalProgress : SCR_SortCompare<UDR_PlayerNetworkComponent>
{
	override static int Compare(UDR_PlayerNetworkComponent left, UDR_PlayerNetworkComponent right)
	{
		return left.m_fTotalProgress < right.m_fTotalProgress;
	}
};