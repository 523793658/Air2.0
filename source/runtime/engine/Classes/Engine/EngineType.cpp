#include "Classes/Engine/EngineType.h"
namespace Air
{
	AttachmentTransformRules AttachmentTransformRules::keepRelativeTransform(EAttachmentRule::KeepRelative, false);

	AttachmentTransformRules AttachmentTransformRules::keepWorldTransform(EAttachmentRule::KeepWorld, false);

	AttachmentTransformRules AttachmentTransformRules::snapToTargetNotIncludingScale(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, false);

	AttachmentTransformRules AttachmentTransformRules::snapToTargetIncludingScale(EAttachmentRule::SnapToTarget, false);

	DetachmentTransformRules DetachmentTransformRules::keepRelativeTranform(EDetachmentRule::KeepRelative, true);
	DetachmentTransformRules DetachmentTransformRules::keepWorldTransform(EDetachmentRule::KeepWorld, true);
}