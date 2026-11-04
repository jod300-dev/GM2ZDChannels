#include "../core/jsonToGDO.h"

#include <Geode/ui/TextInput.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/utils/async.hpp>
#include <Geode/utils/file.hpp>

class ImportPopup : public geode::Popup, TextInputDelegate {
    enum class State {
        SelectJson,
        ParsedJson
    };

    State m_state;
    bool m_allowedExit = true;
    int m_zOrderOffset = 0;
    int m_objsCount = 0;
    float m_drawScale = 1;
    std::string m_objsString;
    std::vector<core::palette::ChannelColor> m_channels;
    std::string m_filename;
    cocos2d::CCPoint m_centerObjectPos;
    geode::TextInput* m_zLayerInput = nullptr;
    cocos2d::CCLabelBMFont* m_parsingText = nullptr;
    CCNode* m_parsedView = nullptr;
    cocos2d::CCMenu* m_parsedViewMenu = nullptr;
    cocos2d::CCLabelBMFont* m_fileLabel = nullptr;
    cocos2d::CCLabelBMFont* m_countLabel = nullptr;
    geode::TextInput* m_drawScaleInput = nullptr;
    CCMenuItemSpriteExtra* m_selectBtn = nullptr;
    static constexpr cocos2d::CCSize m_popupSize = cocos2d::CCSize(275.f, 245.f);
    geode::async::TaskHolder<geode::utils::file::PickResult> m_pickHolder;
    geode::async::TaskHolder<std::optional<core::json2gdo::ParseResult>> m_parseHolder;
protected:
    bool init(cocos2d::CCPoint selectedObjectPos);
    void onHelp(cocos2d::CCObject*);
    void importJSON(cocos2d::CCObject*);
    void onFilePicked(geode::utils::file::PickResult result);
    void parseTextAnimationStep(CCNode* node);
    static arc::Future<std::optional<core::json2gdo::ParseResult>> parseJSON(std::filesystem::path path, core::json2gdo::ParseOptions options);
    void onJSONParsed(std::optional<core::json2gdo::ParseResult> result);
    void textChanged(CCTextInputNode *p0) override;
    void checkAlert(cocos2d::CCObject*);
    void place();
    void keyBackClicked() override;
public:
    static ImportPopup* create(cocos2d::CCPoint selectedObjectPos);
};
