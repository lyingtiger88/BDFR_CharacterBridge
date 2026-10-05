#pragma once

#include "BDFRBodyProfile.h"

class DzJsonWriter;

namespace BDFRDtuExtension
{
    // Writes a namespaced BDFRBodyProfile member into an already-open DTU root object.
    void writeBodyProfile(DzJsonWriter& writer, const BDFRBodyProfile& profile);
}
