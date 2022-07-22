[ComponentEditorProps(category: "UDR", description: "Component to be attached to PlayerController prefab. Used for game mode logic and network.")]
class UDR_PlayerNetworkComponentClass : ScriptComponentClass
{
}

class UDR_PlayerNetworkComponent : ScriptComponent
{
	// Game mode logic
	
	IEntity m_AssignedVehicle;
	protected int m_iPlayerId; // Only valid for server!
	
	[RplProp()]
	bool m_bSpectating;			// Spectating
	
	[RplProp()]
	bool m_bAssignedForRace;	// Will participate in the next race
	
	[RplProp()]
	bool m_bRacingNow;			// Participating in the race now
	
	// Data synchronized from race track logic
	// They are maintained by game mode
	
	[RplProp()]
	int m_iLapCount;		// Our lap count
	
	[RplProp()]
	int m_iPositionInRace;	// Our position among other racers
	
	float m_fTotalProgress;	// Our total distance travelled, including previous laps
	
	int m_iNextWaypoint;
	
	// For moving into vehicle
	protected bool m_bMoveInVehicleRequest = false;
	protected RplId m_MoveInVehicleRplId;
	
	
	void BumpReplication()
	{
		Replication.BumpMe();
	}
	
	override void OnPostInit(IEntity owner)
	{
		//owner.SetFlags(EntityFlags.ACTIVE, true);
		SetEventMask(owner, EntityEvent.FRAME);
	}
	
	//-----------------------------------------------------------------------
	void Init(int playerId)
	{
		m_iPlayerId = playerId;
	}
	
	int GetPlayerId()
	{
		return m_iPlayerId;
	}
	
	//-----------------------------------------------------------------------
	// Playing sounds
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_UiSoundEvent(string soundName)
	{
		SCR_UISoundEntity.SoundEvent(soundName);
	}
	
	void Authority_SendUiSoundEvent(string soundName)
	{
		Rpc(RpcDo_UiSoundEvent, soundName);
	}
	
	//-----------------------------------------------------------------------
	// Moving in vehicle... is a whole serious process
	
	// Must be ccalled by server
	void MoveInVehicle(Vehicle vehicle)
	{
		RplComponent rpl = RplComponent.Cast(vehicle.FindComponent(RplComponent));
		Rpc(RpcDo_MoveInVehicle, rpl.Id());
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_MoveInVehicle(RplId vehId)
	{
		m_bMoveInVehicleRequest = true;
		m_MoveInVehicleRplId = vehId;
	}
	
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
	
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (m_bMoveInVehicleRequest)
		{
			TryMoveInVehicle();
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