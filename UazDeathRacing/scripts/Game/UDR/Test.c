
/*
modded class SCR_CharacterInventoryStorageComponent
{
	override int GetTurretWeaponsList(out array<IEntity> turretWeaponsList, BaseCompartmentSlot compartment)
	{
		int superRetVal = super.GetTurretWeaponsList(turretWeaponsList, compartment);
		if (superRetVal != -1)
			return superRetVal;
		
		// Find weapon manager at vehicle itself, if there is no turret
		
		BaseVehicleNodeComponent vehicleNode = BaseVehicleNodeComponent.Cast(compartment.GetVehicle().FindComponent(BaseVehicleNodeComponent));
		
		if (!vehicleNode)
			return -1;
		
		BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(vehicleNode.FindComponent(BaseWeaponManagerComponent));
		if (!weaponManager)
			return -1;
		
		return weaponManager.GetWeaponsList(turretWeaponsList);
	}
	
	override void OnCompartmentEntered(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		//super.OnCompartmentEntered(targetEntity, manager, mgrID, slotID, move);
		
		BaseCompartmentSlot compartment = manager.FindCompartment(slotID);
		
		if (m_CompartmentAcessComp && m_CompartmentAcessComp.IsGettingIn())
		{
			m_aWeaponQuickSlotsStorage.Clear();
			for (int i = 0; i < 4; i++)
			{
				m_aWeaponQuickSlotsStorage.Insert(m_aQuickSlots[i]);
			}
		}

		array<IEntity> temporaryWeaponsList = {};
		int result = GetTurretWeaponsList(temporaryWeaponsList, compartment);
		
		if (result > 0)
		{
			RemoveItemsFromWeaponQuickSlots();
			
			foreach (IEntity weapon: temporaryWeaponsList)
			{
				if (!weapon)
					continue;
				
				StoreItemToQuickSlot(weapon, -1, true);
			}
		}
		
		SCR_WeaponSwitchingBaseUI.RefreshQuickSlots();
	}
}
*/

modded class ArmaReforgerScripted
{
	override bool OnGameStart()
	{
		super.OnGameStart();
		
		UDR_DebugMenu.Init();
		
		return true;
	}
	
	override void OnUpdate(BaseWorld world, float timeslice)
	{
		super.OnUpdate(world, timeslice);
		
		GetInputManager().ActivateContext("TurretContext", 50);
		
		//GetInputManager().ActivateContext("CarContext", 50);
		
		#ifdef WORKBENCH
		UDR_DebugMenu.UpdateMenus();
		#endif
	}
}

/*
modded class SCR_InventorySlotUI
{
	override bool IsCharacterInTurretCompartment(notnull ChimeraCharacter character)
	{
		CompartmentAccessComponent compAccessComponent = character.GetCompartmentAccessComponent();
		if (!compAccessComponent)
			return false;
		
		//TurretCompartmentSlot turretCompartment = TurretCompartmentSlot.Cast(compAccessComponent.GetCompartment());
		//if (!turretCompartment)
		//	return false;
		
		return true;
	}
}
*/