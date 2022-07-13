class UDR_GameModeClass: SCR_BaseGameModeClass
{
};

protected ref map<int, map<string, RplId>> playersData = new map<int, map<string, RplId>>();

class UDR_GameMode: SCR_BaseGameMode
{
    override void OnPlayerSpawned(int playerId, IEntity controlledEntity)
	{
		super.OnPlayerSpawned(playerId, controlledEntity);
		Print("OnPlayerSpawned");

		vector playerPosition[4];
		controlledEntity.GetWorldTransform(playerPosition);
		
        // List of UDR custom vehicles
		array<ResourceName> vehiclePrefabs = {
			"{1A20D130A03F9CF1}Prefabs/Vehicles/Wheeled/UAZ469/UAZ469_Armed.et",
			"{21C45FA677BCDBDA}Prefabs/Vehicles/Wheeled/M998/M998_Armed.et"
		};
 
		// spawn vehicle
		Resource res = Resource.Load(vehiclePrefabs[0]);
		IEntity newVehicleEntity = GetGame().SpawnEntityPrefab(res, params: (new EntitySpawnParams));
		newVehicleEntity.SetWorldTransform(playerPosition);
		
		// register to destroyed event
		EventHandlerManagerComponent ev = EventHandlerManagerComponent.Cast(newVehicleEntity.FindComponent(EventHandlerManagerComponent));
        ev.RegisterScriptHandler("OnDestroyed", newVehicleEntity, OnVehicleDestroyed);
		
		// save some datas in vehicle, move player in pilot and start the car
		Vehicle veh = Vehicle.Cast(newVehicleEntity);
		UDR_NetworkComponent vehNetComp = UDR_NetworkComponent.Cast(veh.FindComponent(UDR_NetworkComponent));
		Managed test = controlledEntity.FindComponent(RplComponent);
		Print(test);
		RplComponent playerRplComp = RplComponent.Cast(test);
		Print(playerRplComp);
		RplComponent playerNetComp = RplComponent.Cast(controlledEntity.FindComponent(RplComponent));
		RplId vehRplId = Replication.FindId(vehNetComp);
		Print(vehRplId);
		RplId playerRplId = Replication.FindId(playerRplComp);
		Print(playerRplId);
		vehNetComp.SetPlayer(playerRplId);

		SCR_CompartmentAccessComponent compartmentAccessComponent = SCR_CompartmentAccessComponent.Cast(controlledEntity.FindComponent(SCR_CompartmentAccessComponent));
		compartmentAccessComponent.MoveInVehicle(veh, ECompartmentType.Pilot);
		
		CarControllerComponent m_pCarController = CarControllerComponent.Cast(veh.FindComponent(CarControllerComponent));
		m_pCarController.StartEngine();
    }
	
	void OnVehicleDestroyed(IEntity vecEntity)
	{
		RplId playerRplID = UDR_NetworkComponent.Cast(vecEntity.FindComponent(UDR_NetworkComponent)).GetPlayer();
		Print("onDestroyed");
		Print(playerRplID);
		
		UDR_NetworkComponent playerManaged = UDR_NetworkComponent.Cast(Replication.FindItem(playerRplID));
		Print(playerManaged);
		
		if (playerManaged)
			PrintString("did not find player replication");
			return;
		
		IEntity playerEntity = IEntity.Cast(playerManaged.GetOwner());
		Print(playerEntity);
		if (!playerEntity)
			Print("did not find player entity");
			return;
		
		Print("kill him plz");
		Print(playerEntity.FindComponent(SCR_DamageManagerComponent));
		SCR_DamageManagerComponent dmgComp = SCR_DamageManagerComponent.Cast(playerEntity.FindComponent(SCR_DamageManagerComponent));
		dmgComp.Kill();
	}
}