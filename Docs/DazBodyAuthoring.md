# Daz Body Authoring Tool

## Goal

The BDFR Daz Body Authoring Tool lets an artist place muscle and soft-tissue simulation regions in Daz Studio before the character is imported into Unreal.

The goal is not to run the final soft-body solver inside Daz. Daz is used as an authoring environment because the artist can see the character, skeleton and proportions while placing the regions.

## Script

`Tools/DazStudio/BDFR_BodyAuthoring.dsa`

The script uses standard Daz Studio scripting APIs to:

- create a `DzNode` region marker;
- parent the marker in-place to a selected Daz node/bone;
- attach typed user properties to the marker;
- keep the marker non-rendering;
- export all BDFR markers to a `.bdfrbody.json` sidecar.

Daz Studio supports creating scene nodes, parenting children in-place, world/local transforms, user properties and file I/O through its scripting API.

## Authoring muscle regions

For a muscle, select up to two bones/nodes before creating the region.

The script records:

- Anchor A
- Anchor B
- region center marker
- marker local transform relative to Anchor A
- region radii
- mass
- stiffness
- damping
- compliance
- activation scale
- maximum bulge
- collision scale

When two anchors are selected, the marker is initially placed at the midpoint between the two nodes. The artist can then move and rotate it manually.

This creates a useful source representation for a later DynamicBodySystem muscle solver without forcing the Daz skeleton to be modified.

## Authoring soft tissue

For soft tissue, select one bone/node and create a region such as:

- SoftTissue
- FatPad
- Breast
- Glute
- Abdomen
- Custom

The region marker is parented to the selected anchor and can be repositioned with normal Daz transform tools.

## Body profile sidecar

Example:

```json
{
  "schema": "BDFR.BodyProfile",
  "version": 1,
  "units": "cm",
  "coordinateSystem": "DazStudio",
  "characterName": "Genesis9",
  "regions": [
    {
      "name": "Biceps",
      "type": "Muscle",
      "side": "Left",
      "anchorA": "l_upperarm",
      "anchorB": "l_forearm",
      "localPosition": [0, 2, 4],
      "localRotation": [0, 0, 0, 1],
      "radiusCm": [3, 3, 10],
      "stiffness": 0.8,
      "damping": 0.3
    }
  ]
}
```

## Unreal transfer

The sidecar is parsed by `FBDFRDazBodyProfileAdapter` and persisted in `UBDFRCharacterAssetUserData`.

The CharacterBridge FBX importer can explicitly receive a body-profile path or automatically look for:

```text
<CharacterName>.bdfrbody.json
```

next to the DTU file.

This makes body-authoring metadata travel with the character import without requiring custom FBX geometry or extra skeleton bones.

## Coordinate-system policy

The sidecar stores both source local and world transforms and explicitly tags the source coordinate system as `DazStudio`.

The current stage stores those source transforms but does not blindly apply them as Unreal transforms. The later DynamicBodySystem bridge should convert the source marker transform using the same Daz-to-Unreal axis/root conversion used by the character import.

This avoids hard-coding a transform assumption that would fail when Force Front X Axis or another FBX import orientation setting changes.

## Future direct bridge

A later Daz-side CharacterBridge plugin can serialize the same schema directly into the DTU/bridge payload instead of writing a sidecar. The Unreal representation is already independent of transport, so the authoring format will not need to change.
