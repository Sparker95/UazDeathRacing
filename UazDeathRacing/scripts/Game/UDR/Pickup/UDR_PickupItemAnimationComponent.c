[EntityEditorProps(category: "UDR", description: "Animates an entity it's attached to.")]
class UDR_PickupItemAnimationComponentClass : ScriptComponentClass
{
	// prefab properties here
}

//------------------------------------------------------------------------------------------------
/*!
	Class generated via ScriptWizard.
*/
class UDR_PickupItemAnimationComponent : ScriptComponent
{	
	[Attribute("1", UIWidgets.EditBox)]
	protected float m_fRotationAnimPeriod;
	protected float m_fRorationAnimTime = 0;
	
	[Attribute("1", UIWidgets.EditBox)]
	protected float m_fOscillationAnimPeriod;
	protected float m_fOscillationAnimTime = 0;
	
	[Attribute("1", UIWidgets.EditBox)]
	protected float m_fOscillationAnimAmplitude;
	
	protected vector m_vInitialOffset;
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		// Ignore if we are not visible
		if ((owner.GetFlags() & EntityFlags.VISIBLE) == 0)
			return;
		
		Print(string.Format("Visible: %1", true));
		
		// Rotation animation
		m_fRorationAnimTime += timeSlice;
		if (m_fRorationAnimTime > m_fRotationAnimPeriod)
			m_fRorationAnimTime -= m_fRotationAnimPeriod;
		
		vector r;
		r[0] = 360.0 * m_fRorationAnimTime / m_fRotationAnimPeriod;
		owner.SetYawPitchRoll(r);
		
		// Oscillation animation
		m_fOscillationAnimTime += timeSlice;
		if (m_fOscillationAnimTime > m_fOscillationAnimPeriod)
			m_fOscillationAnimTime -= m_fOscillationAnimPeriod;
		vector animationVertOffset;
		animationVertOffset[1] = m_fOscillationAnimAmplitude * Math.Cos(m_fOscillationAnimTime / m_fOscillationAnimPeriod * Math.PI2);
		
		vector transform[4];
		owner.GetLocalTransform(transform);
		transform[3] = m_vInitialOffset + animationVertOffset;
		owner.SetLocalTransform(transform);
		
		
		//Print("UDR_PickupItemAnimationComponent animation", LogLevel.ERROR);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME);
		owner.SetFlags(EntityFlags.ACTIVE, true);
		Print("UDR_PickupItemAnimationComponent OnPostInit", LogLevel.ERROR);
		
		vector transform[4];
		owner.GetLocalTransform(transform);
		m_vInitialOffset = transform[3];
	}

	/*
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
	}

	//------------------------------------------------------------------------------------------------
	void UDR_PickupItemAnimationComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
	}

	//------------------------------------------------------------------------------------------------
	void ~UDR_PickupItemAnimationComponent()
	{
	}
	*/

}
