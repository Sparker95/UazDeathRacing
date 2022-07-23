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
	
	// For moving into vehicle
	protected bool m_bMoveInVehicleRequest = false;
	protected RplId m_MoveInVehicleRplId;
	
	
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