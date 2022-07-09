class UDR_WeaponManagerComponentClass : ScriptComponentClass
{
}

class UDR_WeaponManagerComponent : ScriptComponent
{
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_ServerAddWeapon(int weaponId)
	{
		IEntity vehicleEnt = GetOwner();
		
		array<ResourceName> weaponPrefabs = {
			"{5C3D941FDC76BC95}Prefabs/Weapons/MachineGuns/M60/MG_M60_Mounted_Advanced_Slow.et",
			//"{0746F857CF0EB470}Prefabs/Weapons/MachineGuns/M60/MG_M60_Mounted_Advanced_Fast.et",
			"{58ED978DD73772BE}Prefabs/Weapons/Blaster/Blaster.et"
		};
		
		BaseWeaponManagerComponent weaponMgrComp = BaseWeaponManagerComponent.Cast(vehicleEnt.FindComponent(BaseWeaponManagerComponent));
		array<WeaponSlotComponent> weaponSlots = {};
		weaponMgrComp.GetWeaponsSlots(weaponSlots);
		WeaponSlotComponent slot = weaponSlots[0];
		
		Resource res = Resource.Load(weaponPrefabs[weaponId]);
		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.Parent = vehicleEnt;
		IEntity newWeaponEnt = GetGame().SpawnEntityPrefab(res, params: spawnParams);
		
		RplComponent newRpl = RplComponent.Cast(newWeaponEnt.FindComponent(RplComponent));
		RplId newRplId = newRpl.Id();
		RplId newWeaponEntRplId = Replication.FindId(newWeaponEnt);
		bool newRplIdValid = newRplId.IsValid();
		
		Rpc(RPC_BroadcastSetSlotWeapon, newRplId, false);
		RPC_BroadcastSetSlotWeapon(newRplId, true);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_BroadcastSetSlotWeapon(RplId newWeaponId, bool deletePrevWeapon)
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
		Rpc(RPC_ServerAddWeapon, weaponId);
	}
}