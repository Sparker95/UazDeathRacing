class UDR_HudBase : SCR_InfoDisplay
{
	void UpdateNotificationWidget(TextWidget widget)
	{
		UDR_GameMode gm = UDR_GameMode.Cast(GetGame().GetGameMode());
		PlayerController pc = GetGame().GetPlayerController();
		UDR_PlayerNetworkComponent playerNetworkComp = UDR_PlayerNetworkComponent.Cast(pc.FindComponent(UDR_PlayerNetworkComponent));
		
		// Show notifications
		string notificationsText;
		
		// There can be a fixed notification from game mode
		string staticNotification = gm.GetNotificationText();
		if (!staticNotification.IsEmpty())
		{
			notificationsText = notificationsText + "\n" + staticNotification;
		}
		
		// Sum notifications from player component, line after line
		foreach (UDR_Notification notification : playerNetworkComp.GetNotifications())
		{
			notificationsText = notificationsText + "\n" + notification.m_sText;
		}
		widget.SetText(notificationsText);
	}
	
	// networkEntity - the player for which we will show data
	void UpdateRacePositionWidgets(UDR_PlayerNetworkEntity networkEntity, TextWidget positionWidget, TextWidget lapCountWidget)
	{
		PlayerController pc = GetGame().GetPlayerController();
		UDR_PlayerNetworkComponent playerNetworkComp = UDR_PlayerNetworkComponent.Cast(pc.FindComponent(UDR_PlayerNetworkComponent));
		UDR_GameMode gm = UDR_GameMode.Cast(GetGame().GetGameMode());

		if (networkEntity)
		{
			// Get count of all racers
			int allRacerCount = 0;
			foreach (UDR_PlayerNetworkEntity otherPlayerNetworkEnt : gm.GetAllPlayerNetworkEntities())
			{
				if (!otherPlayerNetworkEnt)
					continue;
				allRacerCount += otherPlayerNetworkEnt.m_bRacingNow;
			}
			
			positionWidget.SetText(string.Format("%1 / %2", networkEntity.m_iPositionInRace+1, allRacerCount));
			
			UDR_RaceTrackLogic currentRaceTrack = gm.GetCurrentRaceTrack();
			lapCountWidget.SetText(string.Format("%1 / %2", networkEntity.m_iLapCount + 1, currentRaceTrack.GetLapCount()));
		}
	}
}