# Forever UpdateFields

Forever UpdateFields are described directly in C++ and serialized from compile-time metadata. No Python or build-time code generator is required.

## Layout

- `FieldDefinition.hpp` contains the generic compile-time descriptors and serializer.
- `Definitions/*.hpp` files are the single source of truth for capture-verified differential fields.
- Small `*.cpp` adapters invoke the definitions and keep `ObjectUpdate.cpp` focused on orchestration.
- `Nested/` contains nested structures such as QuestLog and VisibleItem that are shared by create/update paths.
- `ChangeMask.hpp` owns modern block-mask encoding and byte alignment.
- `WireHelpers.hpp` owns wire-specific helpers such as packed modern GUIDs.
- `Trace.hpp` owns the single-line verification-aware field trace used by every differential serializer.

Adding a verified field normally requires one entry in the relevant `Definitions/*.hpp`. The generic serializer then copies its change bit, observes its parent/group bit and writes the value in definition order.

The old `Generator/` Python files and `Generated/` sources are obsolete and may be deleted after applying this refactor. Their `.cpp` files were previously left as empty stubs so an incremental overlay build could not produce duplicate symbols.

## Current differential coverage

The C++ definition/adaptor layer now covers:

- ObjectData
- UnitData
- ItemData
- ContainerData
- GameObjectData
- DynamicObjectData
- CorpseData
- PlayerData
- ActivePlayerData
- VALUES envelope

ActivePlayerData intentionally exposes only the capture-verified live VALUES fields (coinage, XP, next-level XP and inventory slots). Its CREATE layout remains in the existing create serializer because substantial opaque regions are still being decoded from retail captures.

## PlayerData split

PlayerData differential serialization is kept in `PlayerData.cpp`. Nested `QuestLog` and `VisibleItem` wire formats live under `Nested/` so create/update paths share one implementation. Unknown PlayerData fields remain capture-driven and are not assigned guessed semantics.


## Create serializer split

ObjectData, ItemData, GameObjectData and UnitData CREATE serializers live next to their VALUES serializers. ObjectUpdate.cpp only orchestrates object blocks; wire layout stays in the matching UpdateFields module.

## Stage 9 create-path split

`PlayerData` and `ActivePlayerData` now own both their CREATE and VALUES serializers. The conservative ActivePlayer create-state builder and capture-derived default spans live with `ActivePlayerData` instead of `ObjectUpdate.cpp`. `ObjectUpdate.cpp` only composes the field payload and delegates serialization.

## Forever protocol layout

Forever intentionally uses one shared wire layout across the supported 1.60.x builds. Protocol constants such as object-type bits, VALUES envelope bytes, create fragments and fixed movement header values live in `../ProtocolLayout.hpp`. Do not add build-number branches for these values unless a future capture proves that the Forever wire layout actually changed.

## Verification policy

modern reference schema is a structural reference, not a source of truth for Forever semantics. A field name copied from modern Retail is **reference** until a Forever retail capture or differential test proves it.

Use these labels in comments:

- `[FOREVER-VERIFIED]` - semantics and wire position are proven for Forever.
- `[FOREVER-STRUCTURE]` - byte/bit position, size or ordering is proven, but semantics are not.
- `[REFERENCE]` - semantic name/layout comes from modern reference schema and is not yet proven for Forever.
- `[UNKNOWN]` - no stable semantic identification exists yet.

A semantic-looking C++ identifier by itself never counts as proof. `Definitions/*.hpp` must keep live differential serialization restricted to Forever-verified fields. When a capture disproves a reference name, rename it to a neutral `unknown*` identifier instead of preserving the modern label.

## Verification metadata and field tracing

Every generic field descriptor can now carry `FieldVerification` metadata plus two names:

- the canonical Forever name used by diagnostics;
- an optional modern reference schema reference name.

Reference-only descriptors intentionally use neutral canonical names such as `unknownU32Bit9`; the modern semantic label is retained only in the `reference=` metadata. This prevents debug output from presenting an imported modern reference schema label as proven Forever semantics.

All definition-backed VALUES serializers now emit one single-line `[ForeverDebug][UF][Field]` record for each serialized field. `PlayerData`, which still has capture-specific nested handling, uses the same metadata vocabulary through a small manual metadata table. The record contains scope, bit, array index, canonical name, verification state, optional modern reference name, wire category and value.

Only `VERIFIED` names may be treated as Forever semantics. `STRUCTURE`, `REFERENCE` and `UNKNOWN` entries deliberately use neutral canonical names in diagnostics. Existing semantic-looking storage member names may remain temporarily for source compatibility, but their verification status is defined by `Definitions/*.hpp`, not by the member identifier. This is intended for direct comparison with Forever retail captures while the remaining field map is decoded.

## CREATE verification metadata

The large `UnitData`, `PlayerData` and `ActivePlayerData` CREATE serializers now have explicit ordered metadata tables in `Definitions/*.hpp`. These tables are descriptive and do not change the wire serializer.

Each CREATE entry records serializer order, verification state, a canonical Forever name, an optional modern reference schema reference name, wire type and the conditional branch, if any. `order` is deliberately not called an offset: packed GUIDs, vectors and optional records make absolute byte offsets runtime-dependent.

Semantic-looking storage members inherited from modern Retail may remain temporarily for source compatibility, but unverified CREATE metadata uses neutral canonical names. For example the storage member `mastery` is represented as a `REFERENCE` float slot with `reference=Mastery`; this does not claim that the Forever slot has Mastery semantics.

This gives CREATE and VALUES the same verification vocabulary without changing a single serialized byte. Future capture work should promote individual entries from `REFERENCE`/`STRUCTURE`/`UNKNOWN` to `VERIFIED` only when Forever evidence proves the semantics.


## Neutral storage names

REFERENCE metadata must not leak back into the Forever storage model as if it were verified semantics.
The ActivePlayerData 104-byte post-SkillInfo cluster therefore uses neutral type/position names
(`unknownI32AfterSkill0`, `unknownFloatAfterSkill3`, and so on). Capture-tested combat-stat slots such as block, dodge, parry, melee/ranged/offhand crit and shield block retain semantic names and are VERIFIED. The remaining modern reference schema
labels remain available only through `Definitions::ActivePlayerDataCreateFields::referenceName`.
When a Forever differential proves one of these slots, rename the storage member and promote that
metadata entry to `FieldVerification::Verified` in the same change.


### Stage 19 verification correction

ActivePlayer combat-stat fields that were already behavior/capture verified are intentionally semantic: `blockPercentage`, `dodgePercentage`, `parryPercentage`, `critPercentage`, `rangedCritPercentage`, `offhandCritPercentage`, `shieldBlock`, and `shieldBlockCritPercentage`. Reference-semantic UnitData storage members without established Forever runtime meaning use neutral canonical names; imported semantic labels remain only in `Definitions/UnitData.hpp` metadata.

### Stage 20 UnitData verification corrections

`race`, `classId`, `playerClassId`, `sex`, and `creatureType` retain their established runtime/storage names but are classified as `STRUCTURE`; their wire positions and types are known, while semantic differential proof is intentionally not claimed yet. The owner-visible weapon damage quartet, the StandState/PetTalentPoints/VisFlags/AnimTier quartet, the five-stat cluster, and the SheatheState/PvpFlags/PetFlags/ShapeshiftForm quartet are classified as `VERIFIED` because the existing Forever source comments document capture/self-create evidence for those exact groups. No create wire ordering, types, conditions, or runtime values are changed by this metadata correction.
