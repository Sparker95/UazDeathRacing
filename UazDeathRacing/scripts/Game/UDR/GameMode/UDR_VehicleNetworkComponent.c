[ComponentEditorProps(category: "UDR", description: "Container for datas sync over network")]
class UDR_VehicleNetworkComponentClass : ScriptComponentClass
{
}

class UDR_VehicleNetworkComponent : ScriptComponent
{
	private int playerControllerID;
	
	void SetPlayerControllerID(int playerInt)
	{
		playerControllerID = playerInt;
		Replication.BumpMe();
	}
	int GetPlayerControllerID()
	{
		return playerControllerID;
	}
}