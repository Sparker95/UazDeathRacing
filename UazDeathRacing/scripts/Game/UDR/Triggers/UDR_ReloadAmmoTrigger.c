class UDR_ReloadAmmoTriggerClass : ScriptedGameTriggerEntityClass
{
}

class UDR_ReloadAmmoTrigger : ScriptedGameTriggerEntity
{
	[Attribute("4", UIWidgets.EditBox)];
	protected float m_fRespawnPeriod;
	
	[RplProp(RplGroup.Mandatory, onRplName: "OnRespawnChanged")]
	bool m_bWaitingRespawn = false;
	
	protected float m_fRespawnTimer;
	
	bool m_bAuthority;
	
	override void OnInit(IEntity owner)
	{
		RplComponent rpl = RplComponent.Cast(owner.FindComponent(RplComponent));
		m_bAuthority = rpl.Role() == RplRole.Authority;
		
		if (m_bAuthority)
		{
			SetFlags(EntityFlags.ACTIVE, true);
			SetEventMask(EntityEvent.FRAME);
		}
	}
	
	
	//-------------------------------------------------------------------------------------------
	// Activation
	override void OnActivate(IEntity ent)
	{
		// Ignore on non-server
		if (!m_bAuthority)
			return;
		
		if (ApplyEffect(ent))
			Owner_SetWaitingRespawn(true);
	}
	
	bool ApplyEffect(IEntity ent)
	{		
		UDR_WeaponManagerComponent weaponMgr = UDR_WeaponManagerComponent.Cast(ent.FindComponent(UDR_WeaponManagerComponent));
		
		if (!weaponMgr)
			return false;
		
		weaponMgr.RpcAsk_AddWeapon(UDR_Weapons.BLASTER);
		weaponMgr.Authority_SendPlayPickupSound();
		
		return true;
	}
	
	// This is a small trigger, we msut query it every frame in order not to miss entities
	override void OnFrame(IEntity owner, float timeSlice)
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