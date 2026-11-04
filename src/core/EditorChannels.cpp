#include "./EditorChannels.h"

#include <Geode/Geode.hpp>
#include <algorithm>
#include <set>

using namespace geode::prelude;

namespace {
    // Effect managers that hold the level's color channels. In most GD versions
    // both pointers are the same one, but we handle both just in case.
    std::vector<GJEffectManager*> getEffectManagers(LevelEditorLayer* editor) {
        std::vector<GJEffectManager*> managers;
        auto add = [&managers](GJEffectManager* fx) {
            if (fx && std::find(managers.begin(), managers.end(), fx) == managers.end()) {
                managers.push_back(fx);
            }
        };

        add(editor->m_effectManager);
        if (editor->m_levelSettings) {
            add(editor->m_levelSettings->m_effectManager);
        }
        return managers;
    }

    std::set<int> collectChannelsUsedByObjects(LevelEditorLayer* editor) {
        std::set<int> used;
        if (!editor->m_objects) return used;

        for (auto obj : CCArrayExt<GameObject*>(editor->m_objects)) {
            if (!obj) continue;
            if (obj->m_baseColor) used.insert(obj->m_baseColor->m_colorID);
            if (obj->m_detailColor) used.insert(obj->m_detailColor->m_colorID);
        }
        return used;
    }

    std::vector<int> buildFreeList(
        const std::vector<GJEffectManager*>& managers,
        const std::set<int>& usedByObjects,
        bool checkColorActions
    ) {
        std::vector<int> free;
        for (int id = core::palette::kFirstChannel; id <= core::palette::kLastChannel; id++) {
            if (usedByObjects.contains(id)) continue;

            if (checkColorActions) {
                bool defined = std::any_of(managers.begin(), managers.end(), [id](GJEffectManager* fx) {
                    return fx->colorExists(id);
                });
                if (defined) continue;
            }
            free.push_back(id);
        }
        return free;
    }
}

std::vector<int> core::palette::findFreeChannels(LevelEditorLayer* editor) {
    if (!editor) return {};

    auto managers = getEffectManagers(editor);
    auto usedByObjects = collectChannelsUsedByObjects(editor);

    // Preferred: skip channels that have a color defined OR that objects use
    auto free = buildFreeList(managers, usedByObjects, true);

    // Fallback: if colorExists() reported (almost) everything as defined, only
    // skip the channels that objects really use.
    if (free.size() < 16) {
        auto relaxed = buildFreeList(managers, usedByObjects, false);
        if (relaxed.size() > free.size()) {
            free = std::move(relaxed);
        }
    }
    return free;
}

void core::palette::applyChannels(LevelEditorLayer* editor, const std::vector<ChannelColor>& channels) {
    if (!editor) return;

    for (auto fx : getEffectManagers(editor)) {
        for (const auto& channel : channels) {
            ccColor3B color{channel.color.r, channel.color.g, channel.color.b};

            auto action = fx->getColorAction(channel.id);
            if (!action) {
                action = ColorAction::create(color, false, 0);
                fx->setColorAction(action, channel.id);
            }

            action->m_colorID = channel.id;
            action->m_color = color;
            action->m_fromColor = color;
            action->m_toColor = color;
            action->m_currentOpacity = 1.f;
            action->m_fromOpacity = 1.f;
            action->m_toOpacity = 1.f;
            action->m_blending = false;
            action->m_playerColor = 0;
            action->m_copyID = 0;

            fx->colorActionChanged(action);
        }
        fx->processColors();
    }
}
