//------------------------------------------------------------------------------------------------
class UDR_Faction : SCR_Faction
{
	override bool DoCheckIfFactionFriendly(Faction faction)
	{
		return true;
	}
};
