#include "./ChannelPalette.h"

#include <algorithm>
#include <unordered_map>

namespace {
    using core::palette::Rgb;

    uint32_t packRgb(const Rgb& c) {
        return (uint32_t(c.r) << 16) | (uint32_t(c.g) << 8) | uint32_t(c.b);
    }

    int component(const Rgb& c, int ch) {
        switch (ch) {
            case 0: return c.r;
            case 1: return c.g;
            default: return c.b;
        }
    }

    struct Entry {
        Rgb color;
        uint32_t count;
    };

    struct Range {
        size_t begin, end; // [begin, end) inside the entries vector
    };
}

core::palette::Assignment core::palette::assignChannels(
    const std::vector<Rgb>& objectColors,
    const std::vector<int>& freeChannels
) {
    Assignment result;
    result.objectChannel.assign(objectColors.size(), kFirstChannel);
    if (objectColors.empty() || freeChannels.empty()) {
        return result;
    }

    // 1) Unique colors (in order of first appearance) with their frequency
    std::unordered_map<uint32_t, size_t> seen;
    std::vector<Entry> entries;
    for (const auto& c : objectColors) {
        auto key = packRgb(c);
        auto it = seen.find(key);
        if (it == seen.end()) {
            seen.emplace(key, entries.size());
            entries.push_back({c, 1});
        } else {
            entries[it->second].count++;
        }
    }

    std::unordered_map<uint32_t, int> colorToChannel;
    const size_t channelCount = freeChannels.size();

    if (entries.size() <= channelCount) {
        // 2a) Enough channels: one channel per color, exact colors
        for (size_t i = 0; i < entries.size(); i++) {
            result.channels.push_back({freeChannels[i], entries[i].color});
            colorToChannel.emplace(packRgb(entries[i].color), freeChannels[i]);
        }
    } else {
        // 2b) Not enough channels: median cut down to `channelCount` boxes
        std::vector<Range> boxes{{0, entries.size()}};

        while (boxes.size() < channelCount) {
            int best = -1, bestComponent = 0, bestSpread = -1;

            for (size_t i = 0; i < boxes.size(); i++) {
                const auto& box = boxes[i];
                if (box.end - box.begin < 2) continue;

                int lo[3] = {255, 255, 255}, hi[3] = {0, 0, 0};
                for (size_t j = box.begin; j < box.end; j++) {
                    for (int ch = 0; ch < 3; ch++) {
                        int v = component(entries[j].color, ch);
                        lo[ch] = std::min(lo[ch], v);
                        hi[ch] = std::max(hi[ch], v);
                    }
                }
                for (int ch = 0; ch < 3; ch++) {
                    if (hi[ch] - lo[ch] > bestSpread) {
                        bestSpread = hi[ch] - lo[ch];
                        best = static_cast<int>(i);
                        bestComponent = ch;
                    }
                }
            }

            if (best < 0) break; // nothing left to split

            const Range box = boxes[best];
            std::sort(
                entries.begin() + box.begin, entries.begin() + box.end,
                [bestComponent](const Entry& a, const Entry& b) {
                    return component(a.color, bestComponent) < component(b.color, bestComponent);
                }
            );

            // Split at the weighted median
            uint64_t total = 0;
            for (size_t j = box.begin; j < box.end; j++) total += entries[j].count;

            uint64_t acc = 0;
            size_t mid = box.begin + 1;
            for (size_t j = box.begin; j + 1 < box.end; j++) {
                acc += entries[j].count;
                mid = j + 1;
                if (acc * 2 >= total) break;
            }

            boxes[best] = {box.begin, mid};
            boxes.push_back({mid, box.end});
        }

        // Weighted average color per box -> one channel per box
        for (size_t k = 0; k < boxes.size(); k++) {
            const auto& box = boxes[k];
            uint64_t total = 0, sr = 0, sg = 0, sb = 0;
            for (size_t j = box.begin; j < box.end; j++) {
                total += entries[j].count;
                sr += uint64_t(entries[j].color.r) * entries[j].count;
                sg += uint64_t(entries[j].color.g) * entries[j].count;
                sb += uint64_t(entries[j].color.b) * entries[j].count;
            }

            Rgb avg{
                static_cast<uint8_t>((sr + total / 2) / total),
                static_cast<uint8_t>((sg + total / 2) / total),
                static_cast<uint8_t>((sb + total / 2) / total)
            };
            result.channels.push_back({freeChannels[k], avg});

            for (size_t j = box.begin; j < box.end; j++) {
                colorToChannel.emplace(packRgb(entries[j].color), freeChannels[k]);
            }
        }
    }

    for (size_t i = 0; i < objectColors.size(); i++) {
        result.objectChannel[i] = colorToChannel.at(packRgb(objectColors[i]));
    }
    return result;
}
