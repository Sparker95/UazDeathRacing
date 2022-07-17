/*
Waypoints provide events for race track logic.
*/

class UDR_WaypointClass : ScriptedGameTriggerEntityClass
{
}

class UDR_Waypoint : ScriptedGameTriggerEntity
{	
	// Public event	
	ref ScriptInvoker m_OnActivated = new ScriptInvoker(); // (UDR_Waypoint wp, Vehicle veh) - the vehicle entity which triggered the event
	
	//-------------------------------------------------------------------------------------------
	override void OnInit(IEntity owner)
	{
		// This should work only on server, so don't enable the logic on clients
		if (Replication.IsServer())
		{
			SetFlags(EntityFlags.ACTIVE, true);
			SetEventMask(EntityEvent.POSTFRAME);
		}
	}
	
	//-------------------------------------------------------------------------------------------
	// Activation
	override void OnActivate(IEntity ent)
	{
		if (!Replication.IsServer())
			return;
		
		Vehicle veh = Vehicle.Cast(ent);
		if (!veh)
			return;
		
		m_OnActivated.Invoke(this, veh);
	}
	
	// This is a small trigger, we msut query it every frame in order not to miss entities
	override void EOnPostFrame(IEntity owner, float timeSlice)
	{
		QueryEntitiesInside();
	}
}