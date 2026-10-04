#include <dzplugin.h>
#include "version.h"
#include "BDFRBodyAuthoringAction.h"

DZ_PLUGIN_DEFINITION( "BDFR Body Authoring" );
DZ_PLUGIN_AUTHOR( "BDFR" );
DZ_PLUGIN_VERSION( BDFR_BODY_PLUGIN_MAJOR, BDFR_BODY_PLUGIN_MINOR, BDFR_BODY_PLUGIN_REV, BDFR_BODY_PLUGIN_BUILD );
DZ_PLUGIN_DESCRIPTION(
    "Native Daz Studio body authoring tool for BDFR CharacterBridge. "
    "Authors muscle, soft-tissue and animation-test metadata for Unreal Engine workflows."
);

DZ_PLUGIN_CLASS_GUID( BDFRBodyAuthoringAction, 1A897322-49D2-4C6D-BDF3-5C45E4B143B8 );
