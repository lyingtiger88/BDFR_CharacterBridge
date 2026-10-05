# BDFR Body Profile -> DazToUnreal DTU integration

## Goal

The native Body Authoring plugin owns body-region authoring. The official DazToUnreal bridge owns FBX/DTU transfer.

The integration target is one additional DTU root member named `BDFRBodyProfile`, containing version, gender, Genesis profile, regions and animation clips.

## DazToUnreal hook

The official bridge writes its DTU object in `DzUnrealAction::writeConfiguration()`. The integration point is after standard bridge metadata and before `writer.finishObject()`.

```cpp
writeAllMaterials(...);
writeAllMorphs(writer);
writeMorphLinks(writer);
writeMorphNames(writer);
writeSkeletonData(...);
writeAllDforceInfo(...);
BDFRDtuExtension::writeBodyProfile(writer, selectedFigure);
writer.finishObject();
```

## Rules

- Never replace official DTU keys.
- Use the namespaced root member `BDFRBodyProfile`.
- Units are centimeters and source coordinates are Daz Studio.
- Preserve original Daz bone names in `anchorA` and `anchorB`.
- CharacterBridge performs Daz -> Unreal coordinate conversion.
- If no BDFR profile exists, official DazToUnreal behavior is unchanged.
- Standalone `.bdfbody.json` remains supported.

The public DazBridgeUtils and DazToUnreal repositories are Apache-2.0. If future code is vendored or modified, retain required LICENSE/NOTICE and modification notices.