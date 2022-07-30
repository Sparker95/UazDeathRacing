//----------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------

sealed class UDR_RaceStateNoPlayers : UDR_RaceStateBase
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
	override void OnPlayerRequestJoinRace(UDR_PlayerNetworkComponent playerComp)
	{
		m_GameMode.AssignToNextRace(playerComp);
		m_GameMode.UnassignFromSpectators(playerComp);
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

sealed class UDR_RaceStateOnePlayer : UDR_RaceStateBase
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
	}
	
	//-----------------------------------------------------------------------------
	override void OnPlayerConnected(UDR_PlayerNetworkComponent playerComp)
	{
		// Assign the player to next race
		m_GameMode.AssignToNextRace(playerComp);
	}
	
	//-----------------------------------------------------------------------------
	override void OnPlayerRequestJoinRace(UDR_PlayerNetworkComponent playerComp)
	{
		m_GameMode.AssignToNextRace(playerComp);
		m_GameMode.UnassignFromSpectators(playerComp);
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

sealed class UDR_RaceStatePreparing : UDR_RaceStateBase
{	
	protected const float TIMER_THRESHOLD = 3.0;
	protected float m_fTimer;
	
	//-----------------------------------------------------------------------------
	override void OnStateEnter()
	{
		m_fTimer = TIMER_THRESHOLD;
		
		// Despawn any previous vehicles if they still existed
		m_GameMode.DespawnAllVehicles();
		
		array<UDR_PlayerNetworkComponent> players = m_GameMode.GetNextRacers();
		foreach (UDR_PlayerNetworkComponent player : players)
		{
			m_GameMode.SpawnVehicleAtSpawnPoint(player);
			m_GameMode.UnassignFromSpectators(player);
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
	override void OnPlayerRequestJoinRace(UDR_PlayerNetworkComponent playerComp)
	{
		m_GameMode.AssignToNextRace(playerComp);
		m_GameMode.UnassignFromSpectators(playerComp);
		m_GameMode.SpawnVehicleAtSpawnPoint(playerComp);	// During this stage we can spawn in vehicles for new players
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

sealed class UDR_RaceStateCountdown : UDR_RaceStateBase
{
	protected float m_fTimerSeconds;	// This timer measures seconds
	protected int m_iCountdown;			// Countdown
	
	//-----------------------------------------------------------------------------
	override void OnStateEnter()
	{
		m_fTimerSeconds = 1.0;
		m_iCountdown = 5;
		
		// Player registration is over
		// Assign all players who have registered for next race to the current race
		m_GameMode.UnassignAllFromCurrentRace();
		array<UDR_PlayerNetworkComponent> nextRacePlayers = m_GameMode.GetNextRacers();
		foreach (UDR_PlayerNetworkComponent playerComp : nextRacePlayers)
		{
			m_GameMode.AssignToCurrentRace(playerComp);
		}
	}
	
	//-----------------------------------------------------------------------------
	override void OnStateLeave()
	{
	}
	
	//-----------------------------------------------------------------------------
	override void OnPlayerConnected(UDR_PlayerNetworkComponent playerComp)
	{
		// Assign the player to next race and enable spectator mode
		m_GameMode.AssignToSpectators(playerComp);
		m_GameMode.AssignToNextRace(playerComp);
	}
	
	//-----------------------------------------------------------------------------
	override void OnPlayerRequestJoinRace(UDR_PlayerNetworkComponent playerComp)
	{
		OnPlayerConnected(playerComp); // Same logic as when player connects
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

sealed class UDR_RaceStateRacing : UDR_RaceStateBase
{	
	//-----------------------------------------------------------------------------
	override void OnStateEnter()
	{
		
	}
	
	
	//-----------------------------------------------------------------------------
	override void OnStateLeave()
	{
		// Assign all current racers to the next race
		array<UDR_PlayerNetworkComponent> currentRacers = m_GameMode.GetCurrentRacers();
		foreach (UDR_PlayerNetworkComponent playerComp : currentRacers)
		{
			m_GameMode.AssignToNextRace(playerComp);
		}
	}
	
	//-----------------------------------------------------------------------------
	override void OnPlayerConnected(UDR_PlayerNetworkComponent playerComp)
	{
		// Assign the player to next race and enable spectator mode
		m_GameMode.AssignToSpectators(playerComp);
		m_GameMode.AssignToNextRace(playerComp);
	}
	
	//-----------------------------------------------------------------------------
	override void OnPlayerRequestJoinRace(UDR_PlayerNetworkComponent playerComp)
	{
		OnPlayerConnected(playerComp); // Same logic as when player connects
	}
	
	//-----------------------------------------------------------------------------
	override bool OnUpdate(float timeSlice, out ERaceState outNewState)
	{
		array<UDR_PlayerNetworkComponent> currentRacers = m_GameMode.GetCurrentRacers();
		
		int nCurrentRacers = currentRacers.Count();
		
		if (nCurrentRacers == 0)
		{			
			outNewState = ERaceState.RESULTS; // Everyone has finished or left
			return true;
		}
		/*else if (nCurrentRacers == 1)
		{
			outNewState = ERaceState.ONE_PLAYER;
			return true;
		}*/
		else
		{
			return false; // The race continues
		}
		
		return false;
	}
}




//----------------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------------

sealed class UDR_RaceStateResults : UDR_RaceStateBase
{
	protected float m_fTimerSeconds;
	
	//-----------------------------------------------------------------------------
	override void OnStateEnter()
	{
		m_fTimerSeconds = 6.0;
		
		// Force everyone to spectate, in case the race didn't finish yet
		//m_GameMode.DespawnAllVehicles();
		//array<UDR_PlayerNetworkComponent> currentRacers = m_GameMode.GetCurrentRacers();
		//foreach (UDR_PlayerNetworkComponent playerComp : currentRacers)
		//{
		//	m_GameMode.AssignToSpectators(playerComp);
		//}
	}
	
	
	//-----------------------------------------------------------------------------
	override void OnStateLeave()
	{
	}
	
	//-----------------------------------------------------------------------------
	override bool OnUpdate(float timeSlice, out ERaceState outNewState)
	{
		m_fTimerSeconds -= timeSlice;
		
		if (m_fTimerSeconds <= 0)
		{
			outNewState = ERaceState.PREPARING;
			return true;
		}
		
		return false;
	}
}