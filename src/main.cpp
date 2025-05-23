#include <Geode/Geode.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <Geode/utils/web.hpp>

#include <_a_autoupdate.hpp>

using namespace geode::prelude;

#define public_cast(value, member) [](auto* v) { \
	class FriendClass__; \
	using T = std::remove_pointer<decltype(v)>::type; \
	class FriendeeClass__: public T { \
	protected: \
		friend FriendClass__; \
	}; \
	class FriendClass__ { \
	public: \
		auto& get(FriendeeClass__* v) { return v->member; } \
	} c; \
	return c.get(reinterpret_cast<FriendeeClass__*>(v)); \
}(value)

#define SETTING(type, key_name) Mod::get()->getSettingValue<type>(key_name)

auto enabled = false;

web::WebTask WebRequest_send(web::WebRequest* self, std::string_view method, std::string_view givenUrl) {
    if (enabled and string::contains(givenUrl.data(), "api.geode-sdk.org/v1/mods")) {

        if (givenUrl == "https://api.geode-sdk.org/v1/mods") {
            self->param("status", SETTING(std::string, "status"));
            if (auto par = SETTING(std::string, "gd"); par.size() > 1) self->param("gd", par);
            if (auto par = SETTING(std::string, "platforms"); par.size() > 1) self->param("platforms", par);
            if (auto par = SETTING(std::string, "tags"); par.size() > 1) self->param("tags", par);
            if (auto par = SETTING(std::string, "geode"); par.size() > 1) self->param("geode", par);
            if (auto par = SETTING(std::string, "page"); par.size() > 1) self->param("page", par);
            if (auto par = SETTING(std::string, "per_page"); par.size() > 1) self->param("per_page", par);
        }

        CCNode* version = CCScene::get()->getChildByIDRecursive("version");
        CCLabelBMFont* value_label = typeinfo_cast<CCLabelBMFont*>(
            version ? version->getChildByIDRecursive("value-label") : nullptr
        );
        if (value_label) givenUrl = string::replace(
            givenUrl.data(), "latest",
            string::replace(value_label->getString(), "v", "")
        );

        if (string::contains(givenUrl.data(), "/logo")) {
            auto spliturl = string::split(givenUrl.data(), "/");
            givenUrl = fmt::format(
                "https://gtps.bccst.ru/geode-api-mod-logo-ext.php?id={}",
                spliturl[spliturl.size() - 2]
            );
        }

    }
    log::debug("{}(std::string_view {}, std::string_view {})", __func__, method, givenUrl);
    return self->send(method, givenUrl);
};

Hook* MY_WebRequest_send_HOOK = nullptr;
std::vector<Hook*> WebRequest_send_HOOKS_LIST;
void INIT_WebRequest_send_HOOKS_LIST() {
    if (WebRequest_send_HOOKS_LIST.empty()) {
        for (auto mod : Loader::get()->getAllMods()) for (auto hook : mod->getHooks()) {
            if (string::contains(hook->getDisplayName().data(), "WebRequest::send")) {
                WebRequest_send_HOOKS_LIST.push_back(hook);
            }
        };
    }
}
void TOGGLE_MAIN() {
    //toggle
    if (enabled) {
        enabled = 0;
        //enable not my ones
        INIT_WebRequest_send_HOOKS_LIST();
        for (auto hook : WebRequest_send_HOOKS_LIST) {
            hook->enable();//FUCK_YOU_WHY_IT_WAS_REMOVED_log::debug("result: {}", hook->enable().error_or("is ok."));
            log::debug("hook: {}", hook->getRuntimeInfo().dump());
        }
        //disable my
        MY_WebRequest_send_HOOK->disable();//FUCK_YOU_WHY_IT_WAS_REMOVED_log::debug("result: {}", .error_or("is ok."));
        log::debug("hook: {}", MY_WebRequest_send_HOOK->getRuntimeInfo().dump());
    }
    else {
        enabled = 1;
        //disable not my ones
        INIT_WebRequest_send_HOOKS_LIST();
        for (auto hook : WebRequest_send_HOOKS_LIST) {
            hook->disable();//FUCK_YOU_WHY_IT_WAS_REMOVED_log::debug("result: {}", .error_or("is ok."));
            log::debug("hook: {}", hook->getRuntimeInfo().dump());
        };
        //enable my
        if (MY_WebRequest_send_HOOK) MY_WebRequest_send_HOOK->enable();//FUCK_YOU_WHY_IT_WAS_REMOVED_log::debug("result: {}", .unwrap<std::string>("is ok."));
        else MY_WebRequest_send_HOOK = Mod::get()->hook(
            reinterpret_cast<void*>(getNonVirtual(&web::WebRequest::send)),
            &WebRequest_send, "web::WebRequest::send"_spr, tulip::hook::TulipConvention::Thiscall
        ).unwrap();
        log::debug("hook: {}", MY_WebRequest_send_HOOK->getRuntimeInfo().dump());
    }
}

#include <Geode/modify/CCMenuItem.hpp>
class $modify(ModListButtons, CCMenuItem) {
    $override void activate() {
        CCMenuItem::activate();
        if (this == nullptr) return;
        if (this->m_pListener == nullptr) return;
        if (!typeinfo_cast<CCNode*>(this->m_pListener)) return;

        Ref<CCNode> listener = typeinfo_cast<CCNode*>(this->m_pListener);

        if (listener->getID() == "ModList") {

            if (this->getID() == "sort-button" or this->getID() == "filters-button") {

                findFirstChildRecursive<FLAlertLayer>(CCScene::get(),
                    [](FLAlertLayer* __this) {

                        findFirstChildRecursive<CCMenuItemSpriteExtra>(
                            __this, [](CCMenuItemSpriteExtra* item) {
                                auto image_cast = typeinfo_cast<ButtonSprite*>(item->getNormalImage());
                                if (!image_cast) return false;
                                if (image_cast->m_caption.c_str() != std::string("OK")) return false;
                                auto& org_pfnSelector = item->m_pfnSelector;
                                auto& org_pListener = item->m_pListener;
                                if (org_pListener and org_pfnSelector) CCMenuItemExt::assignCallback<CCMenuItem>(item,
                                    [org_pfnSelector, org_pListener](CCMenuItem* item) {
                                        (org_pListener->*org_pfnSelector)(item);
                                        if (auto reload_btn = typeinfo_cast<CCMenuItem*>(
                                            CCScene::get()->getChildByIDRecursive("reload-button")
                                        )) reload_btn->activate();
                                    }
                                );
                                return true;
                            }
                        );

                        return true;
                    }
                );

            }

            if (this->getID() == "filters-button") {

                findFirstChildRecursive<FLAlertLayer>(CCScene::get(),
                    [](FLAlertLayer* __this) {

                        //is "Search Filters"
                        /*if (!findFirstChildRecursive<CCLabelBMFont>(
                            __this, [](CCLabelBMFont* lbl) {
                                return lbl->getString() == std::string("Search Filters");
                            })) return false;*/

                        auto toggle = CCMenuItemExt::createTogglerWithStandardSprites(0.6,
                            [](auto) {
                                TOGGLE_MAIN();
                            }
                        );
                        toggle->toggle(enabled);
                        toggle->setPosition(20, 20);

                        __this->m_buttonMenu->addChild(toggle);

                        auto Label = CCLabelBMFont::create("Unverified", "bigFont.fnt");
                        Label->setScale(0.40f);
                        Label->setAnchorPoint({ 0.f, 0.5f });
                        Label->setPosition(32, 20);
                        __this->m_buttonMenu->addChild(Label);

                        return true;
                    }
                );

            }

        };
    }
};

#if 0
#endif