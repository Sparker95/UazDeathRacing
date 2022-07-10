[BaseContainerProps()]
class UDR_PickupEffectAmmo : UDR_PickupEffectBase
{
	override bool Authority_ApplyEffect(IEntity ent)
	{
		UDR_WeaponManagerComponent weaponMgr = UDR_WeaponManagerComponent.Cast(ent.FindComponent(UDR_WeaponManagerComponent));
		
		if (!weaponMgr)
			return false;
		
		// Don't pick it up if the vehicle is full on ammo already
		if (weaponMgr.IsFullAmmo())
			return false;
		
		weaponMgr.RpcAsk_AddWeapon(UDR_Weapons.BLASTER);
		weaponMgr.Authority_SendPlayPickupSound();
		
		return true;
	}
}

[BaseContainerProps()]
class UDR_PickupEffectHealth : UDR_PickupEffectBase
{
	override bool Authority_ApplyEffect(IEntity ent)
	{
		UDR_WeaponManagerComponent weaponMgr = UDR_WeaponManagerComponent.Cast(ent.FindComponent(UDR_WeaponManagerComponent));
		
		if (!weaponMgr)
			return false;
		
		// Don't pick it up if the vehicle is full on ammo already
		if (weaponMgr.IsFullAmmo())
			return false;
		
		weaponMgr.RpcAsk_AddWeapon(UDR_Weapons.BLASTER);
		weaponMgr.Authority_SendPlayPickupSound();
		
		return true;
	}
}