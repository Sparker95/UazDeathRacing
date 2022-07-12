class UDR_GameModeClass: SCR_BaseGameModeClass
{
};

protected ref map<int, map<string, RplId>> playersData = new map<int, map<string, RplId>>();

class UDR_GameMode: SCR_BaseGameMode
{
	//[RplProp()]
	protected ref map<int, string> playersta = new map<int, string>();
	
	//[RplProp()]
	protected ref map<int, map<string, RplId>> playersData = new map<int, map<string, RplId>>();
	
    override void OnPlayerSpawned(int playerId, IEntity controlledEntity)
	{
		super.OnPlayerSpawned(playerId, controlledEntity);

		vector playerPosition[4];
		controlledEntity.GetWorldTransform(playerPosition);
		
        // List of UDR custom vehicles
		array<ResourceName> vehiclePrefabs = {
			"{1A20D130A03F9CF1}Prefabs/Vehicles/Wheeled/UAZ469/UAZ469_Armed.et",
			"{21C45FA677BCDBDA}Prefabs/Vehicles/Wheeled/M998/M998_Armed.et"
		};
 
		Resource res = Resource.Load(vehiclePrefabs[0]);
		IEntity newVehicleEntity = GetGame().SpawnEntityPrefab(res, params: (new EntitySpawnParams));
		newVehicleEntity.SetWorldTransform(playerPosition);

		EventHandlerManagerComponent ev = EventHandlerManagerComponent.Cast(newVehicleEntity.FindComponent(EventHandlerManagerComponent));
        ev.RegisterScriptHandler("OnDestroyed", newVehicleEntity, OnVehicleDestroyed);

		Vehicle veh = Vehicle.Cast(newVehicleEntity);
		UDR_WeaponManagerComponent wpnComp = UDR_WeaponManagerComponent.Cast(veh.FindComponent(UDR_WeaponManagerComponent));
		wpnComp.SetServerPlayerID(playerId);
		
		// TODO: seems like we must sync playersData variable
		RplComponent newRpl = RplComponent.Cast(this.FindComponent(RplComponent));
		if (newRpl.Role() == RplRole.Authority) {
			Print("OnPlayerSpawned Is On Server");
		} else {
			Print("OnPlayerSpawned Is On Client");
		}
		//RplId newRplId = newRpl.Id();
		//RplId newWeaponEntRplId = Replication.FindId(this);
		//bool newRplIdValid = newRplId.IsValid();
		
		//Rpc(updatePlayersData, playerId, Replication.FindId(newVehicleEntity), Replication.FindId(controlledEntity));
		map<string, RplId> entities = new map<string, RplId>();
		entities.Set("vehicleEntity", Replication.FindId(newVehicleEntity));
		entities.Set("playerEntity", Replication.FindId(controlledEntity));
		playersData.Set(playerId, entities);
		//Replication.BumpMe();
		//updatePlayersData(playerId, Replication.FindId(newVehicleEntity), Replication.FindId(controlledEntity));
		
		SCR_CompartmentAccessComponent compartmentAccessComponent = SCR_CompartmentAccessComponent.Cast(controlledEntity.FindComponent(SCR_CompartmentAccessComponent));
		compartmentAccessComponent.MoveInVehicle(veh, ECompartmentType.Pilot);
		
		CarControllerComponent m_pCarController = CarControllerComponent.Cast(veh.FindComponent(CarControllerComponent));
		m_pCarController.StartEngine();
		Print(playerId);
		Print(playersData);
		Print(playersData.Get(playerId));
    }
	
	/*[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void updatePlayersData(int playerId, RplId newVehicleRplID, RplId controlledRplID)
	{
		map<string,RplId> entities = new map<string, RplId>();
		entities.Insert("vehicleEntity", IEntity.Cast(Replication.FindItem(newVehicleRplID)));
		entities.Insert("playerEntity", IEntity.Cast(Replication.FindItem(controlledRplID)));
		//this.playersData.Set(playerId, entities);
		//Replication.BumpMe();
	}*/
	
	void OnVehicleDestroyed(IEntity ent)
	{
		int serverPlayerID = UDR_WeaponManagerComponent.Cast(ent.FindComponent(UDR_WeaponManagerComponent)).GetServerPlayerID();
		
		RplComponent newRpl = RplComponent.Cast(this.FindComponent(RplComponent));
		if (newRpl.Role() == RplRole.Authority) {
			Print("OnVehicleDestroyed Is On Server");
		} else {
			Print("OnVehicleDestroyed Is On Client");
		}
		Print(serverPlayerID);
		Print(playersData);
		Print(playersData.Get(serverPlayerID));
		//IEntity playerEntity = this.playersData.Get(serverPlayerID)
		//.Get("playerEntity");
		//Print(playerEntity);
		
		//SCR_DamageManagerComponent.Cast(playerEntity.FindComponent(SCR_DamageManagerComponent)).Kill();
	}
}