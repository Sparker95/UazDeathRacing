modded enum SCR_DebugMenuID
{
	UDR_MENU,
	UDR_SHOW_VEHICLE_PANEL,
	UDR_SHOW_SOUND_PANEL,
	UDR_SHOW_RACE_TRACK_LOGIC_PANEL
}

class UDR_DebugMenu
{
	static const string DEBUG_MENU_NAME = "<Racing>";
	
	static void Init()
	{
		DiagMenu.RegisterMenu(SCR_DebugMenuID.UDR_MENU, DEBUG_MENU_NAME, "");
		
		DiagMenu.RegisterBool(SCR_DebugMenuID.UDR_SHOW_VEHICLE_PANEL, "", "Show Vehicle Panel", DEBUG_MENU_NAME);
		DiagMenu.RegisterBool(SCR_DebugMenuID.UDR_SHOW_SOUND_PANEL, "", "Show Sound Panel", DEBUG_MENU_NAME);
		DiagMenu.RegisterBool(SCR_DebugMenuID.UDR_SHOW_RACE_TRACK_LOGIC_PANEL, "", "Show Race Track Logic Panel", DEBUG_MENU_NAME);
	}
	
	static void DrawVehiclePanel()
	{
		DbgUI.Begin("UDR Vehicle Panel");
		
		// Find components
		PlayerController playerController;
		IEntity playerEntity;
		CharacterControllerComponent characterControllerComp;
		SCR_CompartmentAccessComponent compartmentAccessComp;
		BaseCompartmentSlot compartmentSlot;
		IEntity vehicleEnt;
		TurretControllerComponent turretControllerComp;
		BaseWeaponManagerComponent weaponMgrComp;
		UDR_WeaponManagerComponent udrWeaponMgrComp;
		array<Managed> weaponSlots = {};
		
		
		
		playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;
		playerEntity = playerController.GetControlledEntity();
		if (!playerEntity)
			return;
		characterControllerComp = CharacterControllerComponent.Cast(playerEntity.FindComponent(CharacterControllerComponent));
		compartmentAccessComp = SCR_CompartmentAccessComponent.Cast(playerEntity.FindComponent(SCR_CompartmentAccessComponent));
		if (!compartmentAccessComp)
			return;
		compartmentSlot = compartmentAccessComp.GetCompartment();
		
		if (!compartmentSlot)
			return; // Not in vehicle
		
		vehicleEnt = compartmentSlot.GetOwner();
		
		if (!vehicleEnt)
			return; // WTF?
		
		vehicleEnt.FindComponents(WeaponSlotComponent, weaponSlots);
		turretControllerComp = TurretControllerComponent.Cast(vehicleEnt.FindComponent(TurretControllerComponent));
		weaponMgrComp = BaseWeaponManagerComponent.Cast(vehicleEnt.FindComponent(BaseWeaponManagerComponent));
		udrWeaponMgrComp = UDR_WeaponManagerComponent.Cast(vehicleEnt.FindComponent(UDR_WeaponManagerComponent));
		
		if (!turretControllerComp)
			DbgUI.Text("No TurretControllerComponent found!");
		//if (!weaponMgrComp)
		//	DbgUI.Text("No BaseWeaponManagerComponent found!");
		
		
		//-------------------------------------------------------
		// Weapon selection
		int selectedWeaponId = -1;
		if (DbgUI.Button("Select Weapon 0"))
			selectedWeaponId = 0;
		if (DbgUI.Button("Select Weapon 1"))
			selectedWeaponId = 1;
		if (selectedWeaponId != -1)
		{			
			udrWeaponMgrComp.Owner_RequestAddWeapon(selectedWeaponId);
		}
		
		DbgUI.End();
	}
	
	static void DrawSoundPanel()
	{
		DbgUI.Begin("UDR Sound Panel");
		
		if (DbgUI.Button("Pickup Item"))
		{
			SCR_UISoundEntity.SoundEvent("PICKUP_ITEM");
		}
		
		DbgUI.End();
	}
	
	static void UpdateMenus()
	{
		if (DiagMenu.GetBool(SCR_DebugMenuID.UDR_SHOW_VEHICLE_PANEL))
			DrawVehiclePanel();
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.UDR_SHOW_SOUND_PANEL))
			DrawSoundPanel();
	}
}