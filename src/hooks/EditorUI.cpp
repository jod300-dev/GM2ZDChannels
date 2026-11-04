#include "../layers/ImportPopup.h"

#include <Geode/modify/EditorUI.hpp>
#include <Geode/binding/EditButtonBar.hpp>
#include <Geode/binding/GameManager.hpp>

using namespace geode::prelude;

// This hook adds a custom button
class $modify(MyEditorUI, EditorUI) {

	// Checks does there are selected objects. If so, creates ImportPopup
	void onImgToGD(CCObject*) {
	    auto selectedObjs = CCArrayExt<GameObject*>(this->getSelectedObjects());

		if (selectedObjs.size() != 1) {
			return FLAlertLayer::create(
    			"Info",
                "You have to choose <cg>exactly one</c> object to be the down-left corner of the image!",
    			"OK"
			)->show();
		}

		auto importPopup = ImportPopup::create(selectedObjs[0]->getPosition());
		CCScene::get()->addChild(importPopup);
	}

	void createMoveMenu() {
		// Calling the original function
		EditorUI::createMoveMenu();

		// Creating custom button and adding it to button array
		auto* btn = this->getSpriteButton("geometrize2gd.png"_spr, menu_selector(MyEditorUI::onImgToGD), nullptr, 0.9f);
		m_editButtonBar->m_buttonArray->addObject(btn);

		// Checking settings and updating buttons
		auto rows = GameManager::sharedState()->getIntGameVariable("0049");
		auto columns = GameManager::sharedState()->getIntGameVariable("0050");
		m_editButtonBar->reloadItems(rows, columns);
	}
};
