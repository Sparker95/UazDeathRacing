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
		
		SendUiEventToVehicle(ent, UDR_UISounds.PICKUP_ITEM);
		
		return true;
	}
}

[BaseContainerProps()]
class UDR_PickupEffectHealth : UDR_PickupEffectBase
{
	override bool Authority_ApplyEffect(IEntity ent)
	{
		SCR_VehicleDamageManagerComponent damageMgrComp = SCR_VehicleDamageManagerComponent.Cast(ent.FindComponent(SCR_VehicleDamageManagerComponent));
		
		// Don't pick it up if the vehicle is full on health
		float health = damageMgrComp.GetHealth() / damageMgrComp.GetMaxHealth();
		if (health >= 0.99)
			return false;
		
		// Fortunately it doesn't need to be replicated in any way, replication is already done for us.
		damageMgrComp.FullHeal();
		
		SendUiEventToVehicle(ent, UDR_UISounds.PICKUP_ITEM);
		
		return true;
	}
}