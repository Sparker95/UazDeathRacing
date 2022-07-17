/*
Class for respawnable pickup, handles detection and showing/hiding on clients.
*/

class UDR_PickupClass : ScriptedGameTriggerEntityClass
{
}

class UDR_Pickup : ScriptedGameTriggerEntity
{
	[Attribute("4", UIWidgets.EditBox)];
	protected float m_fRespawnPeriod;
	
	[RplProp(RplGroup.Mandatory, onRplName: "OnRespawnChanged")]
	bool m_bWaitingRespawn = false;
	
	[Attribute("", UIWidgets.Object, "Effect which will be applied")]
	ref UDR_PickupEffectBase m_PickupEffect;
	
	protected float m_fRespawnTimer;
	
	protected bool m_bAuthority;
	
	//-------------------------------------------------------------------------------------------
	override void OnInit(IEntity owner)
	{
		RplComponent rpl = RplComponent.Cast(owner.FindComponent(RplComponent));
		m_bAuthority = rpl.Role() == RplRole.Authority;
		
		if (m_bAuthority)
		{
			SetFlags(EntityFlags.ACTIVE, true);
			SetEventMask(EntityEvent.POSTFRAME);
		}
	}
	
	
	//-------------------------------------------------------------------------------------------
	// Activation
	override void OnActivate(IEntity ent)
	{
		// Ignore on non-server
		if (!m_bAuthority)
			return;
		
		// Try to appy effect. If it couldn't be applied, ignore.
		if (m_PickupEffect.Authority_ApplyEffect(ent))
			Owner_SetWaitingRespawn(true);
	}
	
	// This is a small trigger, we msut query it every frame in order not to miss entities
	override void EOnPostFrame(IEntity owner, float timeSlice)
	{
		if (m_bWaitingRespawn)
		{
			m_fRespawnTimer -= timeSlice;
			if (m_fRespawnTimer <= 0)
				Owner_SetWaitingRespawn(false);
		}
		else
		{
			QueryEntitiesInside();
		}
	}
	
	
	//-------------------------------------------------------------------------------------------
	// Respawn logic and RPCs
	
	// When true, activates timer and hides the pickup
	void Owner_SetWaitingRespawn(bool waitingRespawn)
	{
		if (waitingRespawn)
		{
			m_fRespawnTimer = m_fRespawnPeriod;
		}
		
		m_bWaitingRespawn = waitingRespawn;
		SetVisible(!waitingRespawn);
		Replication.BumpMe();
	}
	
	// Called when respawn flag has changed (on client)
	void OnRespawnChanged()
	{
		SetVisible(!m_bWaitingRespawn);
	}
	
	// Handles local effect
	void SetVisible(bool visible)
	{
		Show(visible); // Sets visible flag recursively
	}
}