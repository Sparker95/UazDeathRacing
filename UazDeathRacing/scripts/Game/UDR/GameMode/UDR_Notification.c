// Small container for sending messages from server to clients. Each notification has a fixed life time.
class UDR_Notification
{
	float m_fTimeEnd;	// World time when it will be deleted
	string m_sText;		// Text
	
	void UDR_Notification(string text, float lifeTime_ms)
	{
		float worldTime = GetGame().GetWorld().GetWorldTime();
		m_sText = text;
		m_fTimeEnd = worldTime + lifeTime_ms;
	}
}