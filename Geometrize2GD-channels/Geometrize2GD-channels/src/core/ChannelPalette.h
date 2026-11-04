#ifndef CHANNELPALETTE_H
#define CHANNELPALETTE_H

#include <cstdint>
#include <vector>

// Pure logic (no Geode dependency): turns the colors of every shape into a small
// palette and assigns one real GD color channel to each palette entry.
namespace core::palette {
    // Channel 1 is NEVER used. Only regular channels (2..999);
    // 1000+ are special channels (BG, ground, P1, P2, ...).
    constexpr int kFirstChannel = 2;
    constexpr int kLastChannel = 999;

    struct Rgb {
        uint8_t r = 0, g = 0, b = 0;
    };

    struct ChannelColor {
        int id;
        Rgb color;
    };

    struct Assignment {
        // Channel assigned to each input color (same order/length as the input).
        std::vector<int> objectChannel;
        // Channels that must be written into the level, with their final color.
        std::vector<ChannelColor> channels;
    };

    // Assigns channels from `freeChannels` (ascending order) to the given colors.
    // Identical colors share one channel. If there are more distinct colors than
    // free channels, the colors are reduced with median cut.
    Assignment assignChannels(const std::vector<Rgb>& objectColors, const std::vector<int>& freeChannels);
}

#endif
