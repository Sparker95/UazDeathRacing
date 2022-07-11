class UDR_GameModeClass: SCR_BaseGameModeClass
{
};

class UDR_GameMode: SCR_BaseGameMode
{
	protected ref map<int, map<string, IEntity>> playersData = new map<int, map<string, IEntity>>();
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
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
		RplId newRplId = newRpl.Id();
		RplId newWeaponEntRplId = Replication.FindId(this);
		bool newRplIdValid = newRplId.IsValid();
		
		
		Rpc(updatePlayersData, playerId, newVehicleEntity, controlledEntity);
		updatePlayersData(playerId, newVehicleEntity, controlledEntity);
		
		SCR_CompartmentAccessComponent compartmentAccessComponent = SCR_CompartmentAccessComponent.Cast(controlledEntity.FindComponent(SCR_CompartmentAccessComponent));
		compartmentAccessComponent.MoveInVehicle(veh, ECompartmentType.Pilot);
		
		CarControllerComponent m_pCarController = CarControllerComponent.Cast(veh.FindComponent(CarControllerComponent));
		m_pCarController.StartEngine();
		Print(playerId);
		Print(this.playersData);
		Print(this.playersData.Get(playerId));
    }
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void updatePlayersData(int playerId, IEntity newVehicleEntity, IEntity controlledEntity)
	{
		map<string,IEntity> entities = new map<string, IEntity>();
		entities.Insert("vehicleEntity", newVehicleEntity);
		entities.Insert("playerEntity", controlledEntity);
		this.playersData.Set(playerId, entities);
	}
	
	void OnVehicleDestroyed(IEntity ent)
	{
		int serverPlayerID = UDR_WeaponManagerComponent.Cast(ent.FindComponent(UDR_WeaponManagerComponent)).GetServerPlayerID();
		Print(serverPlayerID);
		Print(this.playersData);
		Print(this.playersData.Get(serverPlayerID));
		//IEntity playerEntity = this.playersData.Get(serverPlayerID)
		//.Get("playerEntity");
		//Print(playerEntity);
		
		//SCR_DamageManagerComponent.Cast(playerEntity.FindComponent(SCR_DamageManagerComponent)).Kill();
	}
}