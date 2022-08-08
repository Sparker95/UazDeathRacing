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
	
	[Attribute()]
	protected string m_sUiSoundEventName;
	
	// Linked entities are related to this waypoint but are not within its hierarchy.
	// This is used for managing visuals.
	ref array<IEntity> m_aLinkedEntities = {};
	
	//-------------------------------------------------------------------------------------------
	void SetVisible(bool visible)
	{
		if (visible)
		{
			SetFlags(EntityFlags.VISIBLE, true);
			foreach (IEntity ent : m_aLinkedEntities)
				ent.SetFlags(EntityFlags.VISIBLE, true);
		}
		else
		{
			ClearFlags(EntityFlags.VISIBLE, true);
			foreach (IEntity ent : m_aLinkedEntities)
				ent.ClearFlags(EntityFlags.VISIBLE, true);
		}
	}
	
	//-------------------------------------------------------------------------------------------
	string GetUiSoundEventName()
	{
		return m_sUiSoundEventName;
	}
	
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
	
	void RegisterLinkedEntity(IEntity ent)
	{
		m_aLinkedEntities.Insert(ent);
	}
}


// Component class which scales waypoint's subentities according to size of waypoint
class UDR_WaypointAutoScaleComponentClass : ScriptComponentClass {}

class UDR_WaypointAutoScaleComponent : ScriptComponent
{
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
		
		owner.SetScale(10);
	}
	
	override void EOnInit(IEntity owner)
	{
		if (!GetGame().InPlayMode())
			return;
		
		// Scale this object according to trigger size
		UDR_Waypoint parentWp = UDR_Waypoint.Cast(owner.GetParent());
		if (!parentWp)
			return;
		
		// Scale can't be applied to child, so this object must be unparanted
		parentWp.RemoveChild(GetOwner());
		owner.SetOrigin(parentWp.GetOrigin());
		owner.SetScale(2.0 * parentWp.GetSphereRadius());
		
		parentWp.RegisterLinkedEntity(owner);
	}
}