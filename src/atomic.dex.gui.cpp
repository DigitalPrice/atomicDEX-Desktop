/******************************************************************************
 * Copyright © 2013-2019 The Komodo Platform Developers.                      *
 *                                                                            *
 * See the AUTHORS, DEVELOPER-AGREEMENT and LICENSE files at                  *
 * the top-level directory of this distribution for the individual copyright  *
 * holder information and the developer policies on copyright and licensing.  *
 *                                                                            *
 * Unless otherwise agreed in a custom licensing agreement, no part of the    *
 * Komodo Platform software, including this file may be copied, modified,     *
 * propagated or distributed except according to the terms contained in the   *
 * LICENSE file                                                               *
 *                                                                            *
 * Removal or modification of this copyright notice is prohibited.            *
 *                                                                            *
 ******************************************************************************/

#include <imgui.h>
#include <antara/gaming/graphics/component.canvas.hpp>
#include <antara/gaming/event/quit.game.hpp>
#include <antara/gaming/event/key.pressed.hpp>
#include "atomic.dex.gui.hpp"
#include "atomic.dex.gui.widgets.hpp"
#include "atomic.dex.mm2.hpp"

// Helpers
namespace
{
    template<typename T>
    static std::string to_string_precise(const T a_value, const int n = 2)
    {
        std::ostringstream out;
        out.precision(n);
        out << std::fixed << a_value;
        return out.str();
    }

    std::string usd_str(double amt)
    {
        return "$" + to_string_precise(amt) + " USD";
    }
}

namespace
{
    struct coin
    {
        int id;
        std::string code;
        std::string name;

        // Like RICK and MORTY are based on KMD
        std::string base;

        double usd_price;

        bool is_standalone()
        {
            return base == "";
        }
    };

    struct asset
    {
        coin coin;
        double balance;
        std::string address;

        std::string full_name(bool show_base = false)
        {
            return coin.name + " (" + coin.code + ")" +
                   (show_base && !coin.is_standalone() ? ", based on " + coin.base : "");
        }

        double to_usd()
        {
            return balance * coin.usd_price;
        }

        std::string to_usd_str()
        {
            return usd_str(to_usd());
        }
    };

    std::unordered_map<std::string, coin> coins;
    std::unordered_map<std::string, asset> assets;

    static std::string curr_asset_code;
    static int selected = 0;

    void init_coin(int id, const std::string &code, const std::string &name, const std::string &base = "")
    {
        coins[code] = {id, code, name, base, 1337.0};
        assets[code] = {coins[code], 0.0};
    }

    void fill_coins()
    {
        // Clear coins
        {
            coins.clear();
            assets.clear();
        }

        // Fill coins
        {
            int id = 0;
            init_coin(id++, "KMD", "Komodo");
            init_coin(id++, "RICK", "Rick", "KMD");
            init_coin(id++, "MORTY", "Morty", "KMD");
        }
    }

    double get_total_balance()
    {
        double sum{0.0};
        for (auto it = assets.begin(); it != assets.end(); ++it)
            sum += it->second.balance;
        return sum;
    }

    void init() noexcept
    {
        fill_coins();
        curr_asset_code = assets.begin()->second.coin.code;
    }
}

// GUI Draw
namespace
{
    void gui_menubar() noexcept
    {
        if (ImGui::BeginMenuBar()) {
            if (ImGui::MenuItem("Open", "Ctrl+O")) { /* Do stuff */ }
            if (ImGui::MenuItem("Settings", "Ctrl+O")) { /* Do stuff */ }
            ImGui::EndMenuBar();
        }
    }

    void gui_portfolio_coins_list() noexcept
    {
        ImGui::BeginChild("left pane", ImVec2(150, 0), true);
        int i = 0;
        for (auto it = assets.begin(); it != assets.end(); ++it, ++i) {
            auto &asset = it->second;
            if (ImGui::Selectable(asset.full_name().c_str(), selected == i)) {
                selected = i;
                curr_asset_code = asset.coin.code;
            }
        }
        ImGui::EndChild();
    }

    void gui_portfolio_coin_details() noexcept
    {
        // Right
        auto &curr_asset = assets[curr_asset_code];
        ImGui::BeginChild("item view",
                          ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
        {
            ImGui::TextWrapped("%s", curr_asset.full_name(true).c_str());
            ImGui::Separator();
            ImGui::Text(std::string("Balance: %lf %s (%s)").c_str(), curr_asset.balance, curr_asset.coin.code.c_str(),
                        curr_asset.to_usd_str().c_str());
            ImGui::Separator();
            if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None)) {
                if (ImGui::BeginTabItem("Transactions")) {
                    ImGui::Text("Work in progress, transactions will be listed here");
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Receive")) {
                    ImGui::Text("Work in progress, will receive coins here");
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Send")) {
                    ImGui::Text("Work in progress, will send coins here");
                    if (ImGui::Button("Send")) {

                    }
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }
        }
        ImGui::EndChild();
    }

    void gui_portfolio() noexcept
    {
        ImGui::Text("Total Balance: %s", usd_str(get_total_balance()).c_str());

        if (ImGui::Button("Enable a coin")) {

        }

        // Left
        gui_portfolio_coins_list();

        // Right
        ImGui::SameLine();
        gui_portfolio_coin_details();
    }
}

namespace atomic_dex
{
#if defined(ENABLE_CODE_RELOAD_UNIX)
    //! Platform dependent code
    class AtomicDexHotCodeListener : public jet::ILiveListener
    {
    public:
        void onLog(jet::LogSeverity severity, const std::string &message) override
        {
            std::string severityString;
            auto verbosity = loguru::Verbosity_INFO;
            switch (severity) {
                case jet::LogSeverity::kDebug:
                    severityString.append("[D]");
                    verbosity = loguru::Verbosity_INFO;
                    break;
                case jet::LogSeverity::kInfo:
                    severityString.append("[I]");
                    verbosity = loguru::Verbosity_INFO;
                    break;
                case jet::LogSeverity::kWarning:
                    severityString.append("[W]");
                    verbosity = loguru::Verbosity_WARNING;
                    break;
                case jet::LogSeverity::kError:
                    severityString.append("[E]");
                    verbosity = loguru::Verbosity_ERROR;
                    break;
            }
            DVLOG_F(verbosity, "{}", message);
        }
    };
#endif

    void gui::reload_code()
    {
        DVLOG_F(loguru::Verbosity_INFO, "reloading code");
#if defined(ENABLE_CODE_RELOAD_UNIX)
        live_->tryReload();
#endif
    }

    void gui::init_live_coding()
    {
#if defined(ENABLE_CODE_RELOAD_UNIX)
        live_ = jet::make_unique<jet::Live>(jet::make_unique<AtomicDexHotCodeListener>());
        while (!live_->isInitialized()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            live_->update();
        }
        live_->update();
#endif
    }

    void gui::update_live_coding()
    {
#if defined(ENABLE_CODE_RELOAD_UNIX)
        live_->update();
#endif
    }
}

namespace atomic_dex
{
    void gui::on_key_pressed(const ag::event::key_pressed &evt) noexcept
    {
        if (evt.key == ag::input::r && evt.control) {
            reload_code();
        }
    }

    gui::gui(entt::registry &registry, atomic_dex::mm2 &mm2_system) noexcept : system(registry), mm2_system_(mm2_system)
    {
        init_live_coding();
        atomic_dex::style::apply();
        this->dispatcher_.sink<ag::event::key_pressed>().connect<&gui::on_key_pressed>(*this);

        init();
    }

    void gui::update() noexcept
    {
        update_live_coding();


        //! Menu bar
        auto &canvas = entity_registry_.ctx<ag::graphics::canvas_2d>();
        auto[x, y] = canvas.canvas.size;
        auto[pos_x, pos_y] = canvas.canvas.position;

        ImGui::SetNextWindowSize(ImVec2(x, y));
        ImGui::SetNextWindowPos(ImVec2(pos_x, pos_y));
        ImGui::SetNextWindowFocus();
        bool active = true;
        ImGui::Begin("Atomic Dex", &active, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
        if (not active) { this->dispatcher_.trigger<ag::event::quit_game>(0); }

        if (!mm2_system_.is_mm2_initialized()) {
            atomic_dex::widgets::LoadingIndicatorCircle("foo", 30.f, ImVec4(sf::Color::White), ImVec4(sf::Color::Black),
                                                        8, 1.f);
        } else {
            gui_menubar();

            if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None)) {
                if (ImGui::BeginTabItem("Portfolio")) {
                    gui_portfolio();
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Trade")) {
                    ImGui::Text("Work in progress");
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }
        }

        ImGui::End();
    }
}