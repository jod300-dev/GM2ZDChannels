#ifndef COREPARSE_H
#define COREPARSE_H

#include "./ChannelPalette.h"

#include <Geode/binding/GameObject.hpp>
#include <arc/future/Future.hpp>
#include <matjson.hpp>

namespace core::json2gdo {
    namespace {
        constexpr int m_circleID = 497;
    }
    const std::set<int> m_validObjTypes = {5, 3, 4, 32, 8, 16};

    struct ParseOptions {
        cocos2d::CCPoint positionOffset;
        float drawScale;
        int zOrderOffset;
        // Free color channels (2..999, never 1) the parser may assign colors to
        std::vector<int> freeChannels;
    };
    struct ParseResult {
        std::string objects;
        int objectsCount;
        // Channel colors that must be written into the level before placing the objects
        std::vector<palette::ChannelColor> channels;
    };

    ParseResult parse(const matjson::Value &json, ParseOptions options);
    arc::Future<ParseResult> asyncParse(const matjson::Value &json, ParseOptions options);
}

#endif
