[ComponentEditorProps(category: "UDR", description: "Container for datas sync over network")]
class UDR_NetworkComponentClass : ScriptComponentClass
{
	// prefab properties here
}

class UDR_NetworkComponent : ScriptComponent
{
	[RplProp()]
	protected int lapsCompleted;
	[RplProp()]
	protected RplId lastCheckpoint;
	[RplProp()]
	protected RplId vehicle;
	[RplProp()]
	protected RplId player;
	[RplProp()]
	protected RplId spawnPoint;
	
	void SetVehicle(IEntity vehicleEnt)
	{
		this.vehicle = vehicleEnt;
		Replication.BumpMe();
	}
	RplId GetVehicle()
	{
		return this.vehicle;
	}
	
	void SetPlayer(RplId playerEnt)
	{
		this.player = playerEnt;
		Replication.BumpMe();
	}
	RplId GetPlayer()
	{
		return this.player;
	}
}