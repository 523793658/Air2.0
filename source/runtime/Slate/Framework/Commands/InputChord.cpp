#include "Framework/Commands/InputChord.h"
namespace Air
{
	InputChord::ERelationshipType InputChord::getRelationship(const InputChord& otherChord) const
	{
		ERelationshipType relationship = None;
		if (mKey == otherChord.mKey)
		{
			if ((bAlt == otherChord.bAlt) &&
				(bCtrl == otherChord.bCtrl) &&
				(bShift == otherChord.bShift) &&
				(bCmd == otherChord.bCmd))
			{
				relationship = Same;
			}
			else if ((bAlt || !otherChord.bAlt) &&
				(bCtrl || !otherChord.bCtrl) &&
				(bShift || !otherChord.bShift) &&
				(bCmd || !otherChord.bCmd))
			{
				relationship = Masks;
			}
			else if ((!bAlt || otherChord.bAlt) &&
				(!bCtrl || otherChord.bCtrl) &&
				(!bShift || otherChord.bShift) &&
				(!bCmd || otherChord.bCmd))
			{
				relationship = Masked;
			}
		}
		return relationship;
	}
}