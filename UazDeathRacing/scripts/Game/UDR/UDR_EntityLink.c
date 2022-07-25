class UDR_EntityLinkTemplated<Class T> : Managed
{
	[Attribute("", UIWidgets.EditBox)]
	protected string m_sEntityName;
	
	protected T m_Target;
	
	protected static const int COLOR_ERROR = 0xffff0000;
	protected static const int COLOR_GOOD = 0xff00ff80;
	protected static const int COLOR_BACKGROUND = 0x50001005;
		
	void Init()
	{
		m_Target = T.Cast(GetGame().GetWorld().FindEntityByName(m_sEntityName));
		if (!m_Target)
			Print(string.Format("%1: Entity '%2' was not found!", ClassName(), m_sEntityName), LogLevel.ERROR);
	}
	
	T Get()
	{
		return m_Target;
	}
	
	void OnEditorUpdate(IEntity owner)
	{		
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		if (!worldEditor)
			return;
		WorldEditorAPI api = worldEditor.GetApi();
		if (!api)
			return;
		
		IEntitySource targetSrc = api.FindEntityByName(m_sEntityName);
		IEntity targetEnt;
		if (targetSrc)
			targetEnt = api.SourceToEntity(targetSrc);
		
		if (targetEnt && owner)
		{
			// Target is found, draw arrow and text
			
			// Draw arrow
			Shape.CreateArrow(owner.GetOrigin(), targetEnt.GetOrigin(), 2.5, Color.PINK, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER);
			
			// Draw arrow
			bool correctType = targetEnt.Type().IsInherited(T);
			string textStr = m_sEntityName;
			int textColor = COLOR_GOOD;
			if (!correctType)
			{
				textStr = string.Format("'%1' wrong type: %2, expected: %3", m_sEntityName, targetEnt.ClassName(), T);
				textColor = COLOR_ERROR;
			}
				
			vector textPos = 0.25 * owner.GetOrigin() + 0.75 * targetEnt.GetOrigin();
			DebugTextWorldSpace.Create(GetGame().GetWorld(), textStr, DebugTextFlags.ONCE, textPos[0], textPos[1], textPos[2], size: 20.0, color: textColor, bgColor: COLOR_BACKGROUND);
		}
		else if (!targetEnt && !m_sEntityName.IsEmpty())
		{
			// Target is not found, draw error text
			
			string errorText = string.Format("'%1' not found", m_sEntityName);
			vector pos = owner.GetOrigin();
			DebugTextWorldSpace.Create(GetGame().GetWorld(), errorText, DebugTextFlags.ONCE, pos[0], pos[1], pos[2], size: 20.0, color: COLOR_ERROR, bgColor: COLOR_BACKGROUND);
		}
	}
}

[BaseContainerProps()]
class UDR_EntityLink : UDR_EntityLinkTemplated<IEntity> {}

[BaseContainerProps()]
class UDR_EntityLinkVehicle : UDR_EntityLinkTemplated<Vehicle> {}

[BaseContainerProps()]
class UDR_EntityLinkCharacter : UDR_EntityLinkTemplated<ChimeraCharacter> {}