# Architecture

## Overview

BDFR_CharacterBridge separates source-specific character knowledge from Unreal-facing character representation.

```text
Source Character
      |
      v
Source Adapter
      |
      v
Normalized Character Description
      |
+-----+------+----------------+
|            |                |
Skeleton   Animation        Facial
|            |                |
+------------+----------------+
             |
             v
      Unreal Character Assets
```

## Modules

### Core
Defines normalized character metadata, source identity, validation results and shared interfaces.

### Source adapters
Source adapters translate Daz, generic FBX and future DCC-specific conventions into the normalized BDFR representation.

### Skeleton
Responsible for semantic bone mapping, hierarchy interpretation, reference-pose conversion, twist handling and retarget metadata.

### Animation
Responsible for sequence transfer, root motion, pose-space corrections and Unreal retarget integration.

### Facial
Responsible for morph registration, expression naming, source-to-target expression mapping and future ARKit/MetaHuman interoperability.

### Editor
Provides import, diagnostics and conversion tooling. Runtime code should not depend on editor-only import operations.

## Dependency direction

Source-specific adapters may depend on Core interfaces, but Core must not depend on Daz, Blender or MetaHuman-specific logic. This keeps the project extensible and prevents the initial Daz integration from becoming a permanent architectural constraint.
