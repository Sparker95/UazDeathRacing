// Autogenerated by the Generate Class from Layout plugin, version 0.3.0
// Layout file: UI/Lobby/Lobby.layout

class UDR_LobbyWidgets
{
	static const ResourceName s_sLayout = "{E8AA3F7DF6210ACC}UI/Lobby/Lobby.layout";
	ResourceName GetLayout() { return s_sLayout; }

	VerticalLayoutWidget m_PlayerList;

	TextWidget m_RaceTrackNameText;

	TextWidget m_RaceTrackPropertiesText;

	ButtonWidget m_JoinRaceButton;
	SCR_ModularButtonComponent m_JoinRaceButtonComponent;

	ButtonWidget m_SpectateButton;
	SCR_ModularButtonComponent m_SpectateButtonComponent;

	TextWidget m_WarningText;

	bool Init(Widget root)
	{
		m_PlayerList = VerticalLayoutWidget.Cast(root.FindWidget("Overlay0.Content.VerticalLayout0.SectionPlayerList.Overlay0.VerticalLayout0.ScrollSize.Scroll.m_PlayerList"));

		m_RaceTrackNameText = TextWidget.Cast(root.FindWidget("Overlay0.Content.VerticalLayout0.SectionRace.m_RaceTrackNameText"));

		m_RaceTrackPropertiesText = TextWidget.Cast(root.FindWidget("Overlay0.Content.VerticalLayout0.SectionRace.m_RaceTrackPropertiesText"));

		m_JoinRaceButton = ButtonWidget.Cast(root.FindWidget("Overlay0.Content.VerticalLayout0.SectionButtons.HorizontalLayout0.m_JoinRaceButton"));
		m_JoinRaceButtonComponent = SCR_ModularButtonComponent.Cast(m_JoinRaceButton.FindHandler(SCR_ModularButtonComponent));

		m_SpectateButton = ButtonWidget.Cast(root.FindWidget("Overlay0.Content.VerticalLayout0.SectionButtons.HorizontalLayout0.m_SpectateButton"));
		m_SpectateButtonComponent = SCR_ModularButtonComponent.Cast(m_SpectateButton.FindHandler(SCR_ModularButtonComponent));

		m_WarningText = TextWidget.Cast(root.FindWidget("Overlay0.Content.VerticalLayout0.SectionOther.m_WarningText"));

		return true;
	}
}
