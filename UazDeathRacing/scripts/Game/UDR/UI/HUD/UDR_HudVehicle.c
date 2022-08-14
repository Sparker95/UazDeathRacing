class UDR_HudVehicle : UDR_HudBase
{
	protected ref UDR_HudVehicleWidgets widgets = new UDR_HudVehicleWidgets();
	
	protected SCR_VehicleDamageManagerComponent m_CurrentVehicleDamageMgr;
	
	override event void OnStartDraw(IEntity owner)
	{
		super.OnStartDraw(owner);
		
		widgets.Init(GetRootWidget());
		GetRootWidget().SetOpacity(0);
	}
	
	protected const string TEXT_CONTROLS_PCMR		= "LMB - Shoot\nLeft Ctrl - Deploy mine\nHold H - Respawn\nEsc - Lobby";
	protected const string TEXT_CONTROLS_GAMEPAD	= "Y - Shoot\nB - Deploy mine\nHold X - Respawn\nMenu - Lobby";
	
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

		SCR_CompartmentAccessComponent compartmentAccessComp = SCR_CompartmentAccessComponent.Cast(playerEntity.FindComponent(SCR_CompartmentAccessComponent));
		if (!compartmentAccessComp)
		{
			Show(false);
			return;
		}
		
		BaseCompartmentSlot compartmentSlot = compartmentAccessComp.GetCompartment();
		
		if (!compartmentSlot)
		{
			// Not in vehicle
			Show(false);
			return;
		}
		
		IEntity vehicleEnt = compartmentSlot.GetOwner();
		
		if (!vehicleEnt)
		{
			// WTF in compartment but not in vehicle?
			Show(false);
			return;
		}
		
		BaseWeaponManagerComponent weaponMgrComp = BaseWeaponManagerComponent.Cast(vehicleEnt.FindComponent(BaseWeaponManagerComponent));
		UDR_WeaponManagerComponent udrWeaponMgrComp = UDR_WeaponManagerComponent.Cast(vehicleEnt.FindComponent(UDR_WeaponManagerComponent));
		SCR_VehicleDamageManagerComponent damageMgrComp = SCR_VehicleDamageManagerComponent.Cast(vehicleEnt.FindComponent(SCR_VehicleDamageManagerComponent));
		
		// If damage manager has changed
		if (damageMgrComp != m_CurrentVehicleDamageMgr)
		{
			// Subscribe to event of new damage mgr
			if (damageMgrComp)
				damageMgrComp.GetOnDamage().Insert(Callback_OnVehicleDamage);
			
			// Unsubscribe from event of previous damage mgr
			if (m_CurrentVehicleDamageMgr)
				m_CurrentVehicleDamageMgr.GetOnDamage().Remove(Callback_OnVehicleDamage);
			
			m_CurrentVehicleDamageMgr = damageMgrComp;
		}
		
		if (!weaponMgrComp || !damageMgrComp)
		{
			Show(false);
			return;
		}
		
		//-------------------------------------------------------------------------
		// Update values...
		
		UDR_GameMode gm = UDR_GameMode.Cast(GetGame().GetGameMode());
		
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
					if (muzzleComp.IsChamberingPossible())
					{
						int nBarrels = muzzleComp.GetBarrelsCount();
						for (int i = 0; i < nBarrels; i++)
						{
							if (muzzleComp.IsBarrelChambered(i))
								ammoCount++;
						}
					}
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
		
		// Race track
		UDR_PlayerNetworkEntity myNetworkEntity = gm.GetLocalPlayerNetworkEntity();
		if (myNetworkEntity)
			UpdateRacePositionWidgets(myNetworkEntity, widgets.m_PositionText, widgets.m_LapCountText);
		
		// Deployable ammo count
		int deployableAmmoCount = udrWeaponMgrComp.GetDeployableAmmo();
		widgets.m_MinesText.SetText(deployableAmmoCount.ToString());
		
		// Show notifications
		UpdateNotificationWidget(widgets.m_NotificationText);
		
		// Show controls text
		EInputDeviceType lastInputDevice = GetGame().GetInputManager().GetLastUsedInputDevice();
		if (lastInputDevice == EInputDeviceType.GAMEPAD)
			widgets.m_TextControls.SetText(TEXT_CONTROLS_GAMEPAD);
		else
			widgets.m_TextControls.SetText(TEXT_CONTROLS_PCMR);
		
		Show(true);
	}
	
	void Callback_OnVehicleDamage(EDamageType type, float damage, HitZone pHitZone, IEntity instigator, inout vector hitTransform[3], float speed, int colliderID, int nodeID)
	{
		if (damage > 0) {
			widgets.m_BloodVignette1.SetOpacity(1);
			widgets.m_BloodVignette2.SetOpacity(1);
			widgets.m_SuppressionVignette.SetOpacity(1);
			widgets.m_BlackOut.SetOpacity(1);

			WidgetAnimator.PlayAnimation(widgets.m_BloodVignette1, WidgetAnimationType.Opacity, 0, 1);
			WidgetAnimator.PlayAnimation(widgets.m_BloodVignette2, WidgetAnimationType.Opacity, 0, 1);
			WidgetAnimator.PlayAnimation(widgets.m_SuppressionVignette, WidgetAnimationType.Opacity, 0, 1);
			WidgetAnimator.PlayAnimation(widgets.m_BlackOut, WidgetAnimationType.Opacity, 0, 1);
		}
	}
}