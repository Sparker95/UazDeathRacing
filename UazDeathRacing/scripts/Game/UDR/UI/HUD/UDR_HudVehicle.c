class UDR_HudVehicle : SCR_InfoDisplay
{
	protected ref UDR_HudVehicleWidgets widgets = new UDR_HudVehicleWidgets();
	
	
	override event void OnStartDraw(IEntity owner)
	{
		super.OnStartDraw(owner);
		
		widgets.Init(GetRootWidget());
		GetRootWidget().SetOpacity(0);
	}
	
	override event void UpdateValues(IEntity owner, float timeSlice)
	{
		//-------------------------------------------------------------------------
		// Find components, vehicles, weapons, etc...
		PlayerController controller = PlayerController.Cast(owner);
		IEntity playerEntity = controller.GetControlledEntity();
		
		if (!playerEntity)
		{
			Show(false);
			return;
		}
			
		SCR_CompartmentAccessComponent compartmentAccessComp;
		BaseCompartmentSlot compartmentSlot;
		IEntity vehicleEnt;
		BaseWeaponManagerComponent weaponMgrComp;
		UDR_WeaponManagerComponent udrWeaponMgrComp;
		DamageManagerComponent damageMgrComp;
		
		
		compartmentAccessComp = SCR_CompartmentAccessComponent.Cast(playerEntity.FindComponent(SCR_CompartmentAccessComponent));
		if (!compartmentAccessComp)
		{
			Show(false);
			return;
		}
		
		compartmentSlot = compartmentAccessComp.GetCompartment();
		
		if (!compartmentSlot)
		{
			// Not in vehicle
			Show(false);
			return;
		}
		
		vehicleEnt = compartmentSlot.GetOwner();
		
		if (!vehicleEnt)
		{
			// WTF in compartment but not in vehicle?
			Show(false);
			return;
		}
		
		weaponMgrComp = BaseWeaponManagerComponent.Cast(vehicleEnt.FindComponent(BaseWeaponManagerComponent));
		udrWeaponMgrComp = UDR_WeaponManagerComponent.Cast(vehicleEnt.FindComponent(UDR_WeaponManagerComponent));
		damageMgrComp = DamageManagerComponent.Cast(vehicleEnt.FindComponent(DamageManagerComponent));
		
		if (!weaponMgrComp || !damageMgrComp)
		{
			Show(false);
			return;
		}
		
		
		//-------------------------------------------------------------------------
		// Update values...
		
		// Ammo count
		BaseWeaponComponent weaponComp = weaponMgrComp.GetCurrentWeapon();
		int ammoCount = -1;
		if (weaponComp)
		{
			BaseMuzzleComponent muzzleComp = weaponComp.GetCurrentMuzzle();
			if (muzzleComp)
			{
				BaseMagazineComponent magazineComp = muzzleComp.GetMagazine();
				if (magazineComp)
				{
					ammoCount = magazineComp.GetAmmoCount();
					if (muzzleComp.IsChamberingPossible() && muzzleComp.IsBarrelChambered(0))
						ammoCount++;
				}
			}
		}
		if (ammoCount != -1)
			widgets.m_AmmoText.SetText(ammoCount.ToString());
		else
			widgets.m_AmmoText.SetText("-");
		
		// Health
		float health = damageMgrComp.GetHealth() / damageMgrComp.GetMaxHealth();
		float healthPercent = Math.Floor(health * 100.0);
		widgets.m_HealthText.SetText(healthPercent.ToString());
		
		Show(true);
	}
}