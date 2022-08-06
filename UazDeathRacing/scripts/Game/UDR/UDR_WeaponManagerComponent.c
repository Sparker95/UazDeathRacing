enum UDR_Weapons {
	MACHINEGUN,
	BLASTER
}

ref array<ResourceName> UDR_WeaponPrefabs = {
	"{5C3D941FDC76BC95}Prefabs/Weapons/MachineGuns/M60/MG_M60_Mounted_Advanced_Slow.et",
	"{58ED978DD73772BE}Prefabs/Weapons/Blaster/Blaster.et"
};

class UDR_WeaponManagerComponentClass : ScriptComponentClass
{
}

class UDR_WeaponManagerComponent : ScriptComponent
{	
	protected static const int DEPLOYABLE_LIFETIME_MS = 20000;
	
	[Attribute()]
	protected vector m_vDeployablePosition;
	
	[Attribute()]
	protected int m_iDeployableMaxAmmo;
	
	[Attribute()]
	protected int m_iDeployableStartAmmo;
	
	protected int m_iDeployableAmmo;
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_AddWeapon(UDR_Weapons weaponId)
	{
		IEntity vehicleEnt = GetOwner();
		
		BaseWeaponManagerComponent weaponMgrComp = BaseWeaponManagerComponent.Cast(vehicleEnt.FindComponent(BaseWeaponManagerComponent));
		array<WeaponSlotComponent> weaponSlots = {};
		weaponMgrComp.GetWeaponsSlots(weaponSlots);
		WeaponSlotComponent slot = weaponSlots[0];
		
		Resource res = Resource.Load(UDR_WeaponPrefabs[weaponId]);
		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.Parent = vehicleEnt;
		IEntity newWeaponEnt = GetGame().SpawnEntityPrefab(res, params: spawnParams);
		
		RplComponent newRpl = RplComponent.Cast(newWeaponEnt.FindComponent(RplComponent));
		RplId newRplId = newRpl.Id();
		RplId newWeaponEntRplId = Replication.FindId(newWeaponEnt);
		bool newRplIdValid = newRplId.IsValid();
		
		Rpc(RpcDo_BroadcastSetSlotWeapon, newRplId, false);
		RpcDo_BroadcastSetSlotWeapon(newRplId, true);
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_BroadcastSetSlotWeapon(RplId newWeaponId, bool deletePrevWeapon)
	{
		IEntity vehicleEnt = GetOwner();
		
		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(newWeaponId));
		
		if (!rplComp)
			Print("Didn't find new weapon's rpl component on client!", LogLevel.ERROR);
		
		IEntity weaponEnt = rplComp.GetEntity();
		
		BaseWeaponManagerComponent weaponMgrComp = BaseWeaponManagerComponent.Cast(vehicleEnt.FindComponent(BaseWeaponManagerComponent));
		array<WeaponSlotComponent> weaponSlots = {};
		weaponMgrComp.GetWeaponsSlots(weaponSlots);
		WeaponSlotComponent slot = weaponSlots[0];
		
		IEntity prevWeaponEnt = weaponMgrComp.SetSlotWeapon(slot, weaponEnt);
		
		if (prevWeaponEnt && deletePrevWeapon)
			SCR_Global.DeleteEntityAndChildren(prevWeaponEnt);
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	void Owner_RequestAddWeapon(int weaponId)
	{
		Rpc(RpcAsk_AddWeapon, weaponId);
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	// Returns true when this vehicle is full on ammo for both deployable and main weapon
	bool IsMainAmmoFull()
	{
		IEntity vehicleEnt = GetOwner();
		BaseWeaponManagerComponent weaponMgrComp = BaseWeaponManagerComponent.Cast(vehicleEnt.FindComponent(BaseWeaponManagerComponent));
		BaseWeaponComponent weaponComp = weaponMgrComp.GetCurrentWeapon();
		
		bool mainWeaponFull = false;
		
		// Check main weapon ammo
		if (weaponComp)
		{
			BaseMuzzleComponent muzzleComp = weaponComp.GetCurrentMuzzle();
			if (muzzleComp)
			{
				BaseMagazineComponent magazineComp = muzzleComp.GetMagazine();
				if (magazineComp)
				{
					int ammoCount = magazineComp.GetAmmoCount();
					if (muzzleComp.IsChamberingPossible() && muzzleComp.IsBarrelChambered(0))
						ammoCount++;
					
					//return ammoCount == magazineComp.GetMaxAmmoCount();
					mainWeaponFull = ammoCount == 10; // Todo solve it for other guns when we have them
				}
			}
		}
		
		
		return mainWeaponFull;
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	bool IsDeployableAmmoFull()
	{
		return m_iDeployableAmmo == m_iDeployableMaxAmmo;
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	// Deployables
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	bool Owner_RequestFireDeployable()
	{
		// Check if we have enough ammo
		if (m_iDeployableAmmo <= 0)
			return false;
		
		IEntity owner = GetOwner();
		vector myTransform[4]; // aside, up, dir, pos
		owner.GetTransform(myTransform);
		
		
		// Trace move downwards from deployable position to find where to place the mine
		TraceParam tp = new TraceParam();
		tp.Start = owner.CoordToParent(m_vDeployablePosition);
		tp.End = tp.Start - 2.0 * myTransform[1]; // Downwards
		tp.Flags = TraceFlags.ENTS | TraceFlags.WORLD | TraceFlags.ANY_CONTACT;
		tp.Exclude = owner; // Exclude our car
		float traceProgress = GetGame().GetWorld().TraceMove(tp, null);
		if (traceProgress >= 1.0)
			return false; // Invalid trace, didn't hit anything
		
		if (tp.TraceEnt && Vehicle.Cast(tp.TraceEnt))
			return false; // We've hit a car, can't deploy here
		
		// Find the position where to place the deployable
		// It must be placed according to the surface normal
		vector pos = (1 - traceProgress) * tp.Start + traceProgress*tp.End;
		
		vector vUp = tp.TraceNorm;
		vUp.Normalize();
		
		vector vDir;
		if (vector.Distance(vUp, Vector(0, 1, 0)) > 0.01 &&
			vector.Distance(vUp, Vector(0, -1, 0)) > 0.01)
		{
			vDir = (Vector(0, 1, 0)) * vUp;
		}
		else
			vDir = Vector(1, 0, 0);
		vDir.Normalize();
		
		vector vAside = vUp * vDir;
		vAside.Normalize();
		
		m_iDeployableAmmo--;
		
		Rpc(RpcAsk_SyncDeployableAmmo, m_iDeployableAmmo);
		Rpc(RpcAsk_FireDeployable, vAside, vUp, vDir, pos);
		
		return true;
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_FireDeployable(vector vAside, vector vUp, vector vDir, vector pos)
	{
		EntitySpawnParams p = new EntitySpawnParams();
		p.Transform[0] = vAside;
		p.Transform[1] = vUp;
		p.Transform[2] = vDir;
		p.Transform[3] = pos;
		Resource res = Resource.Load("{104B44D37AC166E7}Prefabs/DirtPile/DirtPileDeployable.et");
		IEntity deployableEntity = GetGame().SpawnEntityPrefab(res, params: p);
		
		// Later delete it
		GetGame().GetCallqueue().CallLater(SCR_EntityHelper.DeleteEntityAndChildren, DEPLOYABLE_LIFETIME_MS, false, deployableEntity);
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SyncDeployableAmmo(int ammoCount)
	{
		m_iDeployableAmmo = ammoCount;
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_SetDeployableAmmo(int ammoCount)
	{	
		m_iDeployableAmmo = ammoCount;
		Rpc(RpcAsk_SyncDeployableAmmo, m_iDeployableAmmo);
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	void Authority_SetDeployableAmmo(int ammoCount)
	{
		Rpc(RpcDo_SetDeployableAmmo, ammoCount);
	}
	
	
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	int GetDeployableAmmo() { return m_iDeployableAmmo; }
	int GetDeployableMaxAmmo() { return m_iDeployableMaxAmmo; }
	
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	override void _WB_AfterWorldUpdate(IEntity owner, float timeSlice)
	{
		// Visualize the deployable position
		vector deployablePosWorld = GetOwner().CoordToParent(m_vDeployablePosition);
		Shape.CreateSphere(Color.RED, ShapeFlags.ONCE, deployablePosWorld, 0.05);
	}
	
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_iDeployableAmmo = m_iDeployableStartAmmo;
	}
}