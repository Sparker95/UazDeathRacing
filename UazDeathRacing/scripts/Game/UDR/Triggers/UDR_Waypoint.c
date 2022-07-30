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
	
	[Attribute()]
	ref UDR_EntityLinkWaypoint m_Next;
	
	//-------------------------------------------------------------------------------------------
	override void OnInit(IEntity owner)
	{
		// This should work only on server, so don't enable the logic on clients
		if (Replication.IsServer())
		{
			SetFlags(EntityFlags.ACTIVE, true);
			SetEventMask(EntityEvent.FRAME);
			
			if (GetGame().InPlayMode())
			{
				m_Next.Init();
			}
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
	
	// This is a small trigger, we must query it every frame in order not to miss entities
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		QueryEntitiesInside();
	}
	
	
	override void _WB_AfterWorldUpdate(float timeSlice)
	{
		if (m_Next)
			m_Next.Draw(this);
		
		// Visualize direction
		vector transform[4];
		GetTransform(transform);
		
		vector arrow0 = GetOrigin();
		vector arrow1 = arrow0 + 3.0*transform[2];				
		Shape.CreateArrow(arrow0, arrow1, 1, Color.RED, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER);
	}
}