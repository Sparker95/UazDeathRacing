class UDR_HudRaceResults : UDR_HudBase
{
	protected ref UDR_HudRaceResultsWidgets widgets = new UDR_HudRaceResultsWidgets();
	
	protected bool m_bRefreshTable = true;
	
	override event void OnStartDraw(IEntity owner)
	{
		super.OnStartDraw(owner);
		
		widgets.Init(GetRootWidget());
		GetRootWidget().SetOpacity(0);
	}
	
	override event void UpdateValues(IEntity owner, float timeSlice)
	{
		UDR_GameMode gm = GetGame().GetUdrGameMode();
		
		if (!gm)
		{
			Show(false);
			return;
		}
		
		if (gm.GetRaceState() != ERaceState.RESULTS)
		{
			Show(false);
			m_bRefreshTable = true;
			return;
		}
		
		Show(true);
		
		if (m_bRefreshTable)
		{
			// Delete old entries
			while (widgets.m_PlayerList.GetChildren())
				widgets.m_PlayerList.RemoveChild(widgets.m_PlayerList.GetChildren());
			
			UDR_RaceResultsTable table = gm.GetRaceResults();
			
			UDR_RaceResultsPlayerLineWidgets lineWidgets = new UDR_RaceResultsPlayerLineWidgets();
			foreach (int pos, UDR_RaceResultsEntry entry : table.GetEntries())
			{
				Widget w = GetGame().GetWorkspace().CreateWidgets(UDR_RaceResultsPlayerLineWidgets.s_sLayout, widgets.m_PlayerList);
				lineWidgets.Init(w);
				
				lineWidgets.m_PositionText.SetText((pos + 1).ToString());
				lineWidgets.m_PlayerNameText.SetText(entry.m_sPlayerName);
				
				int hours, minutes, seconds;
				SCR_DateTimeHelper.GetHourMinuteSecondFromSeconds(entry.m_iTotalTime_ms/1000, hours, minutes, seconds);
				string timeFormatted = string.Format("%1:%2", minutes.ToString(2), seconds.ToString(2));
				lineWidgets.m_TimeText.SetText(timeFormatted);
			}
			
			m_bRefreshTable = false;
		}
	}
}