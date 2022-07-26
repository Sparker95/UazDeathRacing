class UDR_EnttityLinkTestClass : GenericEntityClass
{}

class UDR_EnttityLinkTest : GenericEntity
{
	[Attribute()]
	protected ref UDR_EntityLink m_EntityLink0;	// Imagine that we have some entity/component
	
	[Attribute()]
	protected ref UDR_EntityLink m_EntityLink1;	// ... which must find some entity in the world
	
	[Attribute()]
	protected ref UDR_EntityLink m_EntityLink2;	// ... through those attributes
	
	[Attribute()]
	protected ref UDR_EntityLinkVehicle m_VehicleLink;
	
	[Attribute()]
	protected ref UDR_EntityLinkVehicle m_CharacterLink;
	
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (GetGame().InPlayMode())
		{
			m_EntityLink0.Init();
			m_EntityLink1.Init();
			m_EntityLink2.Init();
			m_VehicleLink.Init();
			m_CharacterLink.Init();
		}
	}
	
	override void _WB_AfterWorldUpdate(float timeSlice)
	{
		m_EntityLink0.Draw(this);
		m_EntityLink1.Draw(this);
		m_EntityLink2.Draw(this);
		
		if (m_VehicleLink)
			m_VehicleLink.Draw(this);
		if (m_CharacterLink)
			m_CharacterLink.Draw(this);
	}
	
	//------------------------------------------------------------------------------------------------
	void UDR_EnttityLinkTest(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
		SetFlags(EntityFlags.ACTIVE, true);
	}
}
