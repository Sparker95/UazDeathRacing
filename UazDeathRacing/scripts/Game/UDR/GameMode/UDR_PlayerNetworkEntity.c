/*
Entity created per each player which is meant for streaming player data from server to all other clients.
*/

class UDR_PlayerNetworkEntityClass : GenericEntityClass
{
}

class UDR_PlayerNetworkEntity : GenericEntity
{
	[RplProp()]
	int m_iPlayerId;
	
	[RplProp()]
	int m_iLapCount;		// Our lap count
	
	[RplProp()]
	int m_iPositionInRace;	// Our position among other racers
	
	[RplProp()]
	bool m_bSpectating;			// Spectating
	
	[RplProp()]
	bool m_bAssignedForRace;	// Will participate in the next race
	
	[RplProp()]
	bool m_bRacingNow;			// Participating in the race now
	
	[RplProp()]
	bool m_bFinishedRace;		// In the current race and has finished it
	
	[RplProp()]
	RplId m_AssigedVehicleId;	// Rpl ID of the assigned vehicle
	
	//------------------------------------------------------------------------------------------------
	void BumpReplication()
	{
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	void Init(int playerId)
	{
		m_iPlayerId = playerId;
	}

	//------------------------------------------------------------------------------------------------
	void UDR_PlayerNetworkEntity(IEntitySource src, IEntity parent)
	{
		//SetEventMask(EntityEvent.INIT | EntityEvent.FRAME);
		SetFlags(EntityFlags.ACTIVE, true);
	}

	//------------------------------------------------------------------------------------------------
	void ~UDR_PlayerNetworkEntity()
	{
		UDR_GameMode gm = UDR_GameMode.Cast(GetGame().GetGameMode());
		if (gm)
			gm.UnregisterPlayerNetrowkSyncEntity(this);	// This will unregister itself on client and server
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Called when Item is initialized from replication stream. Carries the data from Master.
	*/
	override protected bool RplLoad(ScriptBitReader reader)
	{
		reader.ReadInt(m_iPlayerId);
		reader.ReadInt(m_iLapCount);
		reader.ReadInt(m_iPositionInRace);
		
		reader.ReadBool(m_bSpectating);
		reader.ReadBool(m_bAssignedForRace);
		reader.ReadBool(m_bRacingNow);
		
		reader.ReadRplId(m_AssigedVehicleId);
		
		UDR_GameMode gm = UDR_GameMode.Cast(GetGame().GetGameMode());
		gm.RegisterPlayerNetrowkSyncEntity(this); // This will register itself on client
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Called when Item is getting replicated from Master to Slave connection. The data will be
	delivered to Slave using RplInit method.
	*/
	override protected bool RplSave(ScriptBitWriter writer)
	{
		writer.WriteInt(m_iPlayerId);
		writer.WriteInt(m_iLapCount);
		writer.WriteInt(m_iPositionInRace);
		
		writer.WriteBool(m_bSpectating);
		writer.WriteBool(m_bAssignedForRace);
		writer.WriteBool(m_bRacingNow);
		
		writer.WriteRplId(m_AssigedVehicleId);
		
		return true;
	}
}
