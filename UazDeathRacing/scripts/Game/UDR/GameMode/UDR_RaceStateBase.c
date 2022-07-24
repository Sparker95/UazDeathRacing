class UDR_RaceStateBase : Managed
{
	UDR_GameMode m_GameMode;
	
	void UDR_RaceStateBase(UDR_GameMode gm)
	{
		m_GameMode = gm;
	}
	
	void OnPlayerConnected(UDR_PlayerNetworkComponent playerComp);
	
	void OnPlayerDisconnected(UDR_PlayerNetworkComponent playerComp);
	
	void OnPlayerRequestJoinRace(UDR_PlayerNetworkComponent playerComp);
	
	void OnStateEnter();
	
	void OnStateLeave();
	
	// When true is returned, game mode will switch to new race state, provided by outNewState
	bool OnUpdate(float timeSlice, out ERaceState outNewState);
}