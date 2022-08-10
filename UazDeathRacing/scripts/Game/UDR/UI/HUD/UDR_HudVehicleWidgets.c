// Autogenerated by the Generate Class from Layout plugin, version 0.3.0
// Layout file: UI/HUD/HudVehicle.layout

class UDR_HudVehicleWidgets
{
	static const ResourceName s_sLayout = "{B97FA0254C5D6D4D}UI/HUD/HudVehicle.layout";
	ResourceName GetLayout() { return s_sLayout; }

	TextWidget m_HealthText;

	TextWidget m_AmmoText;

	TextWidget m_MinesText;

	TextWidget m_LapCountText;

	TextWidget m_PositionText;

	TextWidget m_NotificationText;

	TextWidget m_TextControls;

	bool Init(Widget root)
	{
		m_HealthText = TextWidget.Cast(root.FindWidget("Panel.VerticalLayout0.HudLineHealth.TextRight"));

		m_AmmoText = TextWidget.Cast(root.FindWidget("Panel.VerticalLayout0.HudLineAmmo.TextRight"));

		m_MinesText = TextWidget.Cast(root.FindWidget("Panel.VerticalLayout0.HudLineMines.TextRight"));

		m_LapCountText = TextWidget.Cast(root.FindWidget("RaceProgressPanel.Panel.VerticalLayout0.HudLineLap.TextRight"));

		m_PositionText = TextWidget.Cast(root.FindWidget("RaceProgressPanel.Panel.VerticalLayout0.HudLinePos.TextRight"));

		m_NotificationText = TextWidget.Cast(root.FindWidget("m_NotificationText"));

		m_TextControls = TextWidget.Cast(root.FindWidget("m_TextControls"));

		return true;
	}
}
