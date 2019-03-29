#pragma once
#include "Slate.h"
#include "InputCoreType.h"
#include "GenericPlatform/GenericApplication.h"
namespace Air
{
	struct SLATE_API InputChord
	{
		Key mKey;

		uint32 bShift : 1;

		uint32 bCtrl : 1;

		uint32 bAlt : 1;

		uint32 bCmd : 1;

		enum ERelationshipType
		{
			None,
			Same,
			Masked,
			Masks
		};

		ERelationshipType getRelationship(const InputChord& otherChord) const;

		InputChord()
			:bShift(false),
			bCtrl(false),
			bAlt(false),
			bCmd(false)
		{}

		InputChord(const Key inKey)
			:mKey(inKey),
			bShift(false),
			bCmd(false),
			bCtrl(false),
			bAlt(false)
		{

		}

		InputChord(const Key inKey, const bool bInShift, const bool bInCtrl, const bool bInAlt, const bool bInCmd)
			:mKey(inKey),
			bShift(bInShift),
			bCtrl(bInCtrl),
			bAlt(bInAlt),
			bCmd(bInCmd)
		{

		}

		InputChord(const EModifierKey::Type inModifierKeys, const Key inKey)
			:mKey(inKey),
			bShift((inModifierKeys & EModifierKey::Shift) != 0),
			bCtrl((inModifierKeys & EModifierKey::Control) != 0),
			bAlt((inModifierKeys & EModifierKey::Alt) != 0),
			bCmd((inModifierKeys & EModifierKey::Command) != 0)
		{

		}

		InputChord(const Key inKey, const EModifierKey::Type inModifierKeys)
			:mKey(inKey),
			bShift((inModifierKeys & EModifierKey::Shift) != 0),
			bCtrl((inModifierKeys & EModifierKey::Control) != 0),
			bAlt((inModifierKeys & EModifierKey::Alt) != 0),
			bCmd((inModifierKeys & EModifierKey::Command) != 0)
		{

		}

		InputChord(const InputChord& other)
			:mKey(other.mKey)
			,bShift(other.bShift)
			,bCtrl(other.bCtrl)
			,bAlt(other.bAlt)
			,bCmd(other.bCmd)
		{

		}

		bool operator == (const InputChord& other) const
		{
			return (mKey == other.mKey &&
				bShift == other.bShift &&
				bCtrl == other.bCtrl &&
				bAlt == other.bAlt &&
				bCmd == other.bCmd);
		}

		bool operator != (const InputChord& other) const
		{
			return !(*this == other);
		}

		bool needsControl() const { return bCtrl; }
		bool needsCommand() const { return bCmd; }

		bool needsAlt()const { return bAlt; }

		bool needsShift() const { return bShift; }

		bool hasAnyModifierKeys() const
		{
			return (bAlt || bCtrl || bCmd || bShift);
		}

		bool isValidChord()const
		{
			return (mKey.isValid() && !mKey.isModifierKey());
		}

		bool isValidGestrue() const {
			return isValidChord();
		}

		void set(const InputChord& inTemplate)
		{
			*this = inTemplate;
		}
		friend uint32 getTypeHash(const InputChord& chord)
		{
			return getTypeHash(chord.mKey) ^ (chord.bShift | chord.bCtrl >> 1 | chord.bAlt >> 2 | chord.bCmd >> 3);
		}
	};
}