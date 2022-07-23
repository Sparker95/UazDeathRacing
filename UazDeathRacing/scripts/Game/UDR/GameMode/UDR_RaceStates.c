//----------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------

class UDR_RaceStateNoPlayers : UDR_RaceStateBase
{
	//-----------------------------------------------------------------------------
	override void OnPlayerConnected(UDR_PlayerNetworkComponent playerComp)
	{
		// Assign first player to next race
		m_GameMode.AssignToNextRace(playerComp);
	}
	
	
	//-----------------------------------------------------------------------------
	override void OnPlayerDisconnected(UDR_PlayerNetworkComponent playerComp)
	{
	}
	
	
	//-----------------------------------------------------------------------------
	override void OnStateEnter()
	{
	}
	
	
	//-----------------------------------------------------------------------------
	override void OnStateLeave()
	{
	}
	
	//-----------------------------------------------------------------------------
	override bool OnUpdate(float timeSlice, out ERaceState outNewState)
	{
		array<UDR_PlayerNetworkComponent> players = m_GameMode.GetNextRacers();
		
		int count = players.Count();
		if (count > 0)
		{
			if (count == 1)
				outNewState = ERaceState.ONE_PLAYER;
			else
				outNewState = ERaceState.PREPARING;
			return true;
		}
		
		return false;
	}
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------

class UDR_RaceStateOnePlayer : UDR_RaceStateBase
{	
	
	//-----------------------------------------------------------------------------
	override void OnStateEnter()
	{
		// Spawn our lonely player
		array<UDR_PlayerNetworkComponent> players = m_GameMode.GetNextRacers();
		
		foreach (UDR_PlayerNetworkComponent player : players)
		{
			m_GameMode.SpawnVehicleAtSpawnPoint(player); // Go on, have fun..
		}
	}
	
	
	//-----------------------------------------------------------------------------
	override void OnStateLeave()
	{
		m_GameMode.DespawnAllVehicles();
	}
	
	//-----------------------------------------------------------------------------
	override void OnPlayerConnected(UDR_PlayerNetworkComponent playerComp)
	{
		// Assign the player to next race
		m_GameMode.AssignToNextRace(playerComp);
	}
	
	//-----------------------------------------------------------------------------
	override bool OnUpdate(float timeSlice, out ERaceState outNewState)
	{
		array<UDR_PlayerNetworkComponent> players = m_GameMode.GetNextRacers();
		
		int count = players.Count();
		
		if (count == 1)
			return false;
		else
		{
			if (count == 0)
				outNewState = ERaceState.NO_PLAYERS;
			else
				outNewState = ERaceState.PREPARING;
			return true;
		}
	}
}



//----------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------

class UDR_RaceStatePreparing : UDR_RaceStateBase
{	
	protected const float TIMER_THRESHOLD = 8.0;
	protected float m_fTimer;
	
	//-----------------------------------------------------------------------------
	override void OnStateEnter()
	{
		m_fTimer = TIMER_THRESHOLD;
		
		array<UDR_PlayerNetworkComponent> players = m_GameMode.GetNextRacers();
		foreach (UDR_PlayerNetworkComponent player : players)
		{
			m_GameMode.SpawnVehicleAtSpawnPoint(player);
		}
	}
	
	//-----------------------------------------------------------------------------
	override void OnPlayerConnected(UDR_PlayerNetworkComponent playerComp)
	{
		// Assign the player to next race and spawn his vehicle
		m_GameMode.AssignToNextRace(playerComp);
		m_GameMode.SpawnVehicleAtSpawnPoint(playerComp);
	}
	
	//-----------------------------------------------------------------------------
	override void OnStateLeave()
	{
	}
	
	//-----------------------------------------------------------------------------
	override bool OnUpdate(float timeSlice, out ERaceState outNewState)
	{
		m_fTimer -= timeSlice;
		
		if (m_fTimer <= 0)
		{
			outNewState = ERaceState.COUNTDOWN;
			return true;
		}
		
		return false;
	}
}



//----------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------

class UDR_RaceStateCountdown : UDR_RaceStateBase
{
	protected float m_fTimer;
	
	//-----------------------------------------------------------------------------
	override void OnStateEnter()
	{
		m_fTimer = 3.0;
	}
	
	
	//-----------------------------------------------------------------------------
	override void OnStateLeave()
	{
	}
	
	//-----------------------------------------------------------------------------
	override bool OnUpdate(float timeSlice, out ERaceState outNewState)
	{
		m_fTimer -= timeSlice;
		
		if (m_fTimer <= 0)
		{
			outNewState = ERaceState.RACING;
			return true;
		}
		
		return false;
	}
}



//----------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------

class UDR_RaceStateRacing : UDR_RaceStateBase
{	
	//-----------------------------------------------------------------------------
	override void OnStateEnter()
	{
	}
	
	
	//-----------------------------------------------------------------------------
	override void OnStateLeave()
	{
	}
}




//----------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------

class UDR_RaceStateFinishScreen : UDR_RaceStateBase
{	
	//-----------------------------------------------------------------------------
	override void OnStateEnter()
	{
	}
	
	
	//-----------------------------------------------------------------------------
	override void OnStateLeave()
	{
	}
}