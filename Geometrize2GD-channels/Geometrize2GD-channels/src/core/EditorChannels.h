#ifndef EDITORCHANNELS_H
#define EDITORCHANNELS_H

#include "./ChannelPalette.h"

#include <Geode/binding/LevelEditorLayer.hpp>

namespace core::palette {
    // Returns the color channels (2..999, never 1) that the level isn't using yet,
    // similar to what the "Next Free" button of the color picker does.
    // Must be called from the main thread.
    std::vector<int> findFreeChannels(LevelEditorLayer* editor);

    // Writes the given channel colors into the level's color channels.
    // Must be called from the main thread.
    void applyChannels(LevelEditorLayer* editor, const std::vector<ChannelColor>& channels);
}

#endif
