modded enum SCR_DebugMenuID
{
	UDR_MENU,
	//UDR_SELECT_WEAPON_0,
	//UDR_SELECT_WEAPON_1,
	UDR_SHOW_VEHICLE_PANEL
}

class UDR_DebugMenu
{
	static const string DEBUG_MENU_NAME = "<Racing>";
	
	static void Init()
	{
		DiagMenu.RegisterMenu(SCR_DebugMenuID.UDR_MENU, DEBUG_MENU_NAME, "");
		
		//DiagMenu.RegisterBool(SCR_DebugMenuID.UDR_SELECT_WEAPON_0, "", "Select Weapon 0", DEBUG_MENU_NAME);
		//DiagMenu.RegisterBool(SCR_DebugMenuID.UDR_SELECT_WEAPON_1, "", "Select Weapon 1", DEBUG_MENU_NAME);
		DiagMenu.RegisterBool(SCR_DebugMenuID.UDR_SHOW_VEHICLE_PANEL, "", "Show Vehicle Panel", DEBUG_MENU_NAME);
	}
	
	static void DrawVehiclePanel()
	{
		DbgUI.Begin("Vehicle Panel");
		
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
			/*
			array<ResourceName> weaponPrefabs = {
				"{5C3D941FDC76BC95}Prefabs/Weapons/MachineGuns/M60/MG_M60_Mounted_Advanced_Slow.et",
				"{0746F857CF0EB470}Prefabs/Weapons/MachineGuns/M60/MG_M60_Mounted_Advanced_Fast.et"			
			};
			*/
			/*
			array<IEntity> weapons = {};
			array<BaseWeaponComponent> weaponComps = {};
			weaponMgrComp.GetWeaponsList(weapons);
			weaponComps.Resize(weapons.Count());
			for (int i = 0; i < weapons.Count(); i++)
				weaponComps[i] = BaseWeaponComponent.Cast(weapons[i].FindComponent(BaseWeaponComponent));
			IEntity selectedWeaponEnt = weapons[selectedWeaponId];
			BaseWeaponComponent selectedWeaponComp = BaseWeaponComponent.Cast(selectedWeaponEnt.FindComponent(BaseWeaponComponent));
			*/
			//bool selectWeaponSuccess = turretControllerComp.SelectWeapon(playerEntity, selectedWeaponComp);
			//bool selectWeaponSuccess = characterControllerComp.SelectWeapon(selectedWeaponComp);
			//Print(string.Format("Select weapon success: %1", selectWeaponSuccess));
			
			/*
			WeaponSlotComponent slot = WeaponSlotComponent.Cast(weaponSlots[0]);
			Resource res = Resource.Load(weaponPrefabs[selectedWeaponId]);
			EntitySpawnParams spawnParams = new EntitySpawnParams();
			spawnParams.Parent = vehicleEnt;
			IEntity newWeaponEnt = GetGame().SpawnEntityPrefab(res, params: spawnParams);
			IEntity prevWeaponEnt = weaponMgrComp.SetSlotWeapon(slot, newWeaponEnt);
			if (prevWeaponEnt)
				SCR_Global.DeleteEntityAndChildren(prevWeaponEnt);
			*/
			
			udrWeaponMgrComp.Owner_RequestAddWeapon(selectedWeaponId);
		}
		
		DbgUI.End();
	}
	
	//void Update()
	//{
	//	
	//}
	
	static void UpdateMenus()
	{
		if (DiagMenu.GetBool(SCR_DebugMenuID.UDR_SHOW_VEHICLE_PANEL))
			DrawVehiclePanel();
	}
}