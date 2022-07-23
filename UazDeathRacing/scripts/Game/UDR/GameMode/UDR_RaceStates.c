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
			if (player.m_AssignedVehicle == null)
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
	protected const float TIMER_THRESHOLD = 3.0;
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
	protected float m_fTimerSeconds;	// This timer measures seconds
	protected int m_iCountdown;			// Countdown
	
	//-----------------------------------------------------------------------------
	override void OnStateEnter()
	{
		m_fTimerSeconds = 1.0;
		m_iCountdown = 5;
	}
	
	
	//-----------------------------------------------------------------------------
	override void OnStateLeave()
	{
	}
	
	//-----------------------------------------------------------------------------
	override bool OnUpdate(float timeSlice, out ERaceState outNewState)
	{
		m_fTimerSeconds -= timeSlice;
		
		bool switchState = false;
		
		if (m_fTimerSeconds <= 0)
		{
			m_GameMode._print(string.Format("Countdown: %1", m_iCountdown));
			
			string soundEventName;
			string countdownText;
			if (m_iCountdown == 0)
			{
				countdownText = "GO!";
				soundEventName = UDR_UISounds.RACE_START;
				
				// Switch to next state
				outNewState = ERaceState.RACING;
				switchState = true;
			}
			else
			{
				countdownText = m_iCountdown.ToString();
				soundEventName = UDR_UISounds.RACE_COUNTDOWN;
			}
			
			m_GameMode.BroadcastUiSoundEvent(soundEventName);
			m_GameMode.BroadcastNotification(countdownText, 900.0);
		
			m_iCountdown--;	
			m_fTimerSeconds = 1.0;
		}
		
		return switchState;
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