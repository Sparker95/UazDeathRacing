[ComponentEditorProps(category: "UDR", description: "Component to be attached to PlayerController prefab. Used for game mode logic and network.")]
class UDR_PlayerNetworkComponentClass : ScriptComponentClass
{
}

class UDR_PlayerNetworkComponent : ScriptComponent
{
	void BumpReplication()
	{
		Replication.BumpMe();
	}
	
	//-----------------------------------------------------------------------
	// Game mode logic
	
	IEntity m_AssignedVehicle;
	
	[RplProp()]
	bool m_bSpectator;		// Spectating
	
	[RplProp()]
	bool m_bAssignedForRace;	// Will race in the next race
	
	[RplProp()]
	bool m_bRacingNow;			// Participating in the race now
	
	//-----------------------------------------------------------------------
	// Properties synchronized from race track logic
	// They are maintained by game mode
	
	[RplProp()]
	int m_iLapCount;		// Our lap count
	
	[RplProp()]
	int m_iPositionInRace;	// Our position among other racers
	
	//[RplProp()]
	float m_fTotalProgress;	// Our total distance travelled, including previous laps
	
	int m_iNextWaypoint;
	
	//-----------------------------------------------------------------------
	// Logic for sound playing
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_UiSoundEvent(string soundName)
	{
		SCR_UISoundEntity.SoundEvent(soundName);
	}
	
	void Authority_SendUiSoundEvent(string soundName)
	{
		Rpc(RpcDo_UiSoundEvent, soundName);
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