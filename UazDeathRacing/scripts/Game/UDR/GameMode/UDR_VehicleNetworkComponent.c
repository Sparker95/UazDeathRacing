[ComponentEditorProps(category: "UDR", description: "Container for datas sync over network")]
class UDR_VehicleNetworkComponentClass : ScriptComponentClass
{
}

class UDR_VehicleNetworkComponent : ScriptComponent
{
	protected int m_PlayerId;
	
	void Init(int playerId)
	{
		m_PlayerId = playerId;
	}
	
	int GetPlayerId()
	{
		return m_PlayerId;
	}
}