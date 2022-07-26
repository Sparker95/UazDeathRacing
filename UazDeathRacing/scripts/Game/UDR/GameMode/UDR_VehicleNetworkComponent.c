[ComponentEditorProps(category: "UDR", description: "Container for datas sync over network")]
class UDR_VehicleNetworkComponentClass : ScriptComponentClass
{
}

class UDR_VehicleNetworkComponent : ScriptComponent
{
	protected int playerControllerID;
	
	void Init(int playerId)
	{
		playerControllerID = playerId;
	}
	
	int GetPlayerId()
	{
		return playerControllerID;
	}
}