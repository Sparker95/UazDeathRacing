class UDR_RaceResultsEntry : JsonApiStruct
{
	int m_iPlayerId;
	string m_sPlayerName;
	int m_iTotalTime_ms;
	
	void UDR_RaceResultsEntry()
	{
		RegV("m_iPlayerId");
		RegV("m_sPlayerName");
		RegV("m_iTotalTime_ms");
	}
}

class UDR_RaceResultsTable : JsonApiStruct
{
	protected ref array<ref UDR_RaceResultsEntry> m_aEntries = {};
	
	void Add(int playerId, string playerName, float totalTime_ms)
	{
		UDR_RaceResultsEntry e = new UDR_RaceResultsEntry();
		e.m_iPlayerId	= playerId;
		e.m_sPlayerName	= playerName;
		e.m_iTotalTime_ms	= Math.Round(totalTime_ms);
		m_aEntries.Insert(e);
	}
	
	void Clear()
	{
		m_aEntries.Clear();
	}
	
	int GetCount()
	{
		return m_aEntries.Count();
	}
	
	array<ref UDR_RaceResultsEntry> GetEntries()
	{
		return m_aEntries;
	}
	
	void UDR_RaceResultsTable()
	{
		RegV("m_aEntries");
	}
}