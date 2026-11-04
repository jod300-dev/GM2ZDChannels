#include "./jsonToGDO.h"

#include <Geode/utils/async.hpp>
#include <Geode/utils/StringBuffer.hpp>
#include <algorithm>

core::json2gdo::ParseResult core::json2gdo::parse(const matjson::Value &json, ParseOptions options) {
    // Everything needed to write one object, collected first so that the colors
    // can be turned into channels before generating the object string.
    struct Shape {
        float posX, posY, scaleX, scaleY, rotation;
    };
    std::vector<Shape> shapes;
    std::vector<palette::Rgb> colors;
    int objsCount = 0;

    auto toByte = [](double value) {
        return static_cast<uint8_t>(std::clamp(value, 0.0, 255.0) + 0.5);
    };

    for (auto obj : json) {
        // Avoiding objects with zero score
        auto objScore = obj["score"].asDouble();
        if (!objScore || objScore.unwrap() <= 0) {
            continue;
        }

        // Setting and initializing default properties if there is missing some
        float posX = options.positionOffset.x;
        float posY = options.positionOffset.y;
        float scaleX = 1.f, scaleY = 1.f, rotation = 0.f;
        auto redResult = obj["color"][0].asDouble();
        auto greenResult = obj["color"][1].asDouble();
        auto blueResult = obj["color"][2].asDouble();

        // Validating types
        if (auto objType = obj["type"].asInt()) {
            if (!m_validObjTypes.contains(objType.unwrap()))
                continue;
            else
                objsCount++;
        }

        if (auto posXResult = obj["data"][0].asDouble()) {
            posX += posXResult.unwrap() * options.drawScale;
        }
        if (auto posYResult = obj["data"][1].asDouble()) {
            posY += posYResult.unwrap() * options.drawScale;
        }
        if (auto scaleXResult = obj["data"][2].asDouble()) {
            scaleX = scaleXResult.unwrap() * options.drawScale / 4 * 0.16;
        }
        if (auto scaleYResult = obj["data"][3].asDouble()) {
            scaleY = scaleYResult.unwrap() * options.drawScale / 4 * 0.16;
        } else {
            scaleY = scaleX;
        }
        if (auto rotationResult = obj["data"][4].asDouble()) {
            rotation = -rotationResult.unwrap();
        }

        // Parsing colors (plain RGB, no HSV)
        palette::Rgb color;
        if (redResult && greenResult && blueResult) {
            color = {
                toByte(redResult.unwrap()),
                toByte(greenResult.unwrap()),
                toByte(blueResult.unwrap())
            };
        }

        shapes.push_back({posX, posY, scaleX, scaleY, rotation});
        colors.push_back(color);
    }

    // Every distinct color gets its own color channel (never channel 1)
    auto assignment = palette::assignChannels(colors, options.freeChannels);

    // Parsing objects to gd format and increasing Z Order.
    // 21 = base color channel, 22 = detail color channel. HSV is not used anymore.
    geode::utils::StringBuffer<> objsBuf;
    for (size_t i = 0; i < shapes.size(); i++) {
        const auto& shape = shapes[i];
        int channel = assignment.objectChannel[i];

        objsBuf.append(
            "1,{},2,{},3,{},128,{},129,{},6,{},21,{},22,{},25,{},372,1;",
            m_circleID, shape.posX, shape.posY, shape.scaleX, shape.scaleY, shape.rotation,
            channel, channel, options.zOrderOffset
        );
        options.zOrderOffset++;
    }

    return ParseResult {
        .objects = objsBuf.str(),
        .objectsCount = objsCount,
        .channels = std::move(assignment.channels)
    };
}

arc::Future<core::json2gdo::ParseResult> core::json2gdo::asyncParse(const matjson::Value &json, ParseOptions options) {
    auto handle = geode::async::runtime().spawnBlocking<ParseResult>(
        [json, options]() {
            return parse(json, options);
        }
    );

    auto result = co_await handle;
    co_return result;
}
