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
- export all BDFR markers to a `.bdfrbody.json` sidecar;
- manage Walk, Run and custom animation test clips;
- apply a selected Daz animation/pose preset through the Content Manager;
- play/pause the selected movement preview while inspecting region placement.

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

## Walk / Run animation test clips

The main Daz tool now initializes two clip slots:

```text
Walk
Run
```

These are test/authoring slots rather than bundled third-party animation assets. Use **Add Clip** and choose a Daz animation or pose preset (`.duf`, `.dsf`, `.dsa`) or an importable BVH file. If the selected type is Walk or Run and its default slot has no file assigned yet, the slot is replaced instead of duplicated.

The tool provides:

- **Add Clip** — choose a clip type, name and source file.
- **Remove Clip** — remove the selected clip from the BDFR authoring profile.
- **Apply Selected Clip** — selects the authored character and merges/applies the preset through Daz Studio's Content Manager.
- **Play / Pause Preview** — runs the Daz scene playback so muscle/soft-tissue marker placement can be inspected in motion.

Walk and Run default to looping during preview. Custom clips can choose their own loop setting.

The animation clip list is stored on a non-rendering `BDFR_BodyAuthoringRoot` node in the Daz scene, so reopening the script does not lose the list.

The exported sidecar contains:

```json
"animationClips": [
  {
    "name": "Walk",
    "type": "Walk",
    "sourceFile": "C:/Animations/Walk.duf",
    "loop": true
  },
  {
    "name": "Run",
    "type": "Run",
    "sourceFile": "C:/Animations/Run.duf",
    "loop": true
  }
]
```

CharacterBridge parses these records into `FBDFRBodyAnimationClip`. The source file is authoring metadata; Unreal does not assume that a Daz `.duf` file is directly playable. A later animation-transfer step can associate the Walk/Run test role with the corresponding imported/retargeted Unreal Animation Sequence.

## Body profile sidecar

Example:

```json
{
  "schema": "BDFR.BodyProfile",
  "version": 1,
  "units": "cm",
  "coordinateSystem": "DazStudio",
  "characterName": "Genesis9",
  "animationClips": [
    {
      "name": "Walk",
      "type": "Walk",
      "sourceFile": "C:/Animations/Walk.duf",
      "loop": true
    },
    {
      "name": "Run",
      "type": "Run",
      "sourceFile": "C:/Animations/Run.duf",
      "loop": true
    }
  ],
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
