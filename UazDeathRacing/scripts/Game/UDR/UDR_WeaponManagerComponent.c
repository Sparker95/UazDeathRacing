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
	// need to clean this, a component only for UDR datas maybe ?
	protected int serverPlayerID;
	
	void SetServerPlayerID(int playerID)
	{
		this.serverPlayerID = playerID;
	}
	
	int GetServerPlayerID()
	{
		return this.serverPlayerID;
	}

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
	
	void Owner_RequestAddWeapon(int weaponId)
	{
		Rpc(RpcAsk_AddWeapon, weaponId);
	}
	
	
	// Returns true when this vehicle is full on ammo
	bool IsFullAmmo()
	{
		IEntity vehicleEnt = GetOwner();
		BaseWeaponManagerComponent weaponMgrComp = BaseWeaponManagerComponent.Cast(vehicleEnt.FindComponent(BaseWeaponManagerComponent));
		BaseWeaponComponent weaponComp = weaponMgrComp.GetCurrentWeapon();
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
					
					return ammoCount == magazineComp.GetMaxAmmoCount();
				}
			}
		}
		
		return false;
	}
}