#include "weather/weather_screen.h"

#include "input_manager.h"

#include <algorithm>
#include <chrono>
#include <fmt/format.h>

WeatherScreen::WeatherScreen(WeatherClient& client, SettingsStore& settings) : client_(client), settings_(settings) {
    build();
}

void WeatherScreen::show() {
    lv_scr_load(root_);
    selectCityId(settings_.settings().activeCityId);
}

void WeatherScreen::selectCity(int index, bool fetch) {
    const int cityCount = static_cast<int>(weatherLocations().size());
    cityIndex_ = (index % cityCount + cityCount) % cityCount;
    dayIndex_ = 0;
    updateStaticData();
    updateForecastSelection();
    settings_.setActiveCity(weatherLocations()[static_cast<size_t>(cityIndex_)].id);
    settings_.save();
    updateModalRows();
    if (fetch) refresh();
}

void WeatherScreen::selectCityId(const std::string& id, bool fetch) {
    const auto& locations = weatherLocations();
    for (size_t index = 0; index < locations.size(); ++index) {
        if (id == locations[index].id) {
            selectCity(static_cast<int>(index), fetch);
            return;
        }
    }
    selectCity(0, fetch);
}

long long WeatherScreen::nowSeconds() const {
    return std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

void WeatherScreen::refresh(bool manual) {
    const auto& city = weatherLocations()[static_cast<size_t>(cityIndex_)];
    if (!hasCurrentData_) {
        if (auto cached = settings_.cached(city.id, nowSeconds(), 24 * 60 * 60)) {
            applyWeather(*cached);
            showNotice("Mostrando dato guardado");
        } else {
            setLoading(true);
        }
    }
    ++latestRequestId_;
    lastRefreshAt_ = nowSeconds();
    // The central notice sits directly above the forecast cards, so reserve it
    // for actionable messages (offline/cache). Refresh state belongs beside
    // the normal update timestamp instead of competing with weather data.
    showNotice("");
    lv_label_set_text(updatedLabel_, manual ? "Actualizando..." : "Actualizando...");
    client_.fetch(city);
}

std::vector<int> WeatherScreen::selectableCities() const {
    std::vector<int> result;
    if (showingCatalog_) {
        for (size_t index = 0; index < weatherLocations().size(); ++index) {
            result.push_back(static_cast<int>(index));
        }
        return result;
    }
    for (const auto& id : settings_.settings().favorites) {
        for (size_t index = 0; index < weatherLocations().size(); ++index) {
            if (id == weatherLocations()[index].id) result.push_back(static_cast<int>(index));
        }
    }
    return result;
}

void WeatherScreen::updateModalRows() {
    if (!modalTitle_) return;
    lv_label_set_text(modalTitle_, showingCatalog_ ? "CATÁLOGO  [IZQ: FAVORITOS]" : "FAVORITOS  [DER: CATÁLOGO]");
    const auto cities = selectableCities();
    int selected = 0;
    for (size_t index = 0; index < cities.size(); ++index) {
        if (cities[index] == cityIndex_) selected = static_cast<int>(index);
    }
    const auto page = fmt::format("{} / {}", selected + 1, cities.size());
    lv_label_set_text(modalPageLabel_, page.c_str());
    for (size_t row = 0; row < modalRows_.size(); ++row) {
        const int source = selected - 1 + static_cast<int>(row);
        const bool visible = source >= 0 && source < static_cast<int>(cities.size());
        if (!visible) {
            modalRowCityIndices_[row] = -1;
            lv_obj_add_flag(modalRows_[row], LV_OBJ_FLAG_HIDDEN);
            continue;
        }
        lv_obj_clear_flag(modalRows_[row], LV_OBJ_FLAG_HIDDEN);
        const int city = cities[static_cast<size_t>(source)];
        const auto& location = weatherLocations()[static_cast<size_t>(city)];
        modalRowCityIndices_[row] = city;
        lv_label_set_text(modalRowNames_[row], location.name);
        const auto detail = fmt::format("{}{}", location.country,
                                        settings_.isFavorite(location.id) ? "  [F]" : "");
        lv_label_set_text(modalRowDetails_[row], detail.c_str());
        lv_obj_set_style_bg_opa(modalRows_[row], city == cityIndex_ ? LV_OPA_40 : LV_OPA_TRANSP, LV_PART_MAIN);
    }
}

void WeatherScreen::update() {
    if (lastRefreshAt_ > 0 && nowSeconds() - lastRefreshAt_ >= 30 * 60) refresh();
    while (auto result = client_.takeResult()) {
        if (std::string(result->location.id) != weatherLocations()[static_cast<size_t>(cityIndex_)].id) continue;
        if (result->status == WeatherStatus::Ready) {
            applyWeather(*result);
        } else if (hasCurrentData_) {
            setLoading(false);
            if (auto cached = settings_.cached(result->location.id, nowSeconds(), 24 * 60 * 60)) applyWeather(*cached);
            showNotice("Sin conexión - mostrando dato guardado");
        } else {
            setLoading(false);
            lv_label_set_text(temperatureLabel_, "-- C");
            lv_label_set_text(conditionLabel_, "Sin datos");
            lv_label_set_text(rangeLabel_, "No se pudieron cargar los datos");
            showNotice("Sin conexión");
        }
    }
}

void WeatherScreen::handleAction(int actionValue) {
    const auto action = static_cast<Action>(actionValue);
    if (modalVisible_) {
        const auto cities = selectableCities();
        const auto selected = std::find(cities.begin(), cities.end(), cityIndex_);
        const int position = selected == cities.end() ? 0 : static_cast<int>(std::distance(cities.begin(), selected));
        if (action == Action::Up && !cities.empty()) {
            selectCity(cities[(position + static_cast<int>(cities.size()) - 1) % static_cast<int>(cities.size())], false);
            setModalVisible(true);
        }
        if (action == Action::Down && !cities.empty()) {
            selectCity(cities[(position + 1) % static_cast<int>(cities.size())], false);
            setModalVisible(true);
        }
        if (action == Action::Left || action == Action::Right) {
            showingCatalog_ = action == Action::Right;
            updateModalRows();
        }
        if (action == Action::Confirm) {
            setModalVisible(false);
            selectCity(cityIndex_);
        }
        if (action == Action::Favorite) {
            const auto& id = weatherLocations()[static_cast<size_t>(cityIndex_)].id;
            const bool changed = settings_.isFavorite(id) ? settings_.removeFavorite(id) : settings_.addFavorite(id);
            if (changed) settings_.save();
            updateModalRows();
        }
        if (!showingCatalog_ && (action == Action::MoveFavoriteUp || action == Action::MoveFavoriteDown)) {
            const auto& id = weatherLocations()[static_cast<size_t>(cityIndex_)].id;
            if (settings_.moveFavorite(id, action == Action::MoveFavoriteUp ? -1 : 1)) settings_.save();
            updateModalRows();
        }
        if (action == Action::Back) setModalVisible(false);
        return;
    }

    if (detailVisible_) {
        if (action == Action::Left) {
            dayIndex_ = (dayIndex_ + 4) % 5;
            updateForecastSelection();
            updateDetail();
        }
        if (action == Action::Right) {
            dayIndex_ = (dayIndex_ + 1) % 5;
            updateForecastSelection();
            updateDetail();
        }
        if (action == Action::Confirm || action == Action::Back) setDetailVisible(false);
        return;
    }

    const auto favorites = selectableCities();
    const auto found = std::find(favorites.begin(), favorites.end(), cityIndex_);
    const int position = found == favorites.end() ? 0 : static_cast<int>(std::distance(favorites.begin(), found));
    if (action == Action::Up && !favorites.empty()) {
        selectCity(favorites[(position + static_cast<int>(favorites.size()) - 1) % static_cast<int>(favorites.size())]);
    }
    if (action == Action::Down && !favorites.empty()) {
        selectCity(favorites[(position + 1) % static_cast<int>(favorites.size())]);
    }
    if (action == Action::Left) {
        dayIndex_ = (dayIndex_ + 4) % 5;
        updateForecastSelection();
    }
    if (action == Action::Right) {
        dayIndex_ = (dayIndex_ + 1) % 5;
        updateForecastSelection();
    }
    if (action == Action::Confirm) setDetailVisible(true);
    if (action == Action::Back) setModalVisible(true);
    if (action == Action::Refresh) refresh(true);
    if (action == Action::Favorite) {
        const auto& id = weatherLocations()[static_cast<size_t>(cityIndex_)].id;
        if (settings_.isFavorite(id)) settings_.removeFavorite(id);
        else settings_.addFavorite(id);
        settings_.save();
        updateModalRows();
    }
}

void WeatherScreen::setModalVisible(bool visible) {
    modalVisible_ = visible;
    if (visible) {
        lv_obj_clear_flag(modalBackdrop_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(modal_, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(modalBackdrop_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(modal_, LV_OBJ_FLAG_HIDDEN);
        showingCatalog_ = false;
    }
    updateModalRows();
}

void WeatherScreen::setDetailVisible(bool visible) {
    detailVisible_ = visible;
    if (visible) {
        updateDetail();
        lv_obj_move_foreground(detail_);
        lv_obj_clear_flag(detail_, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(detail_, LV_OBJ_FLAG_HIDDEN);
    }
}

void WeatherScreen::cityButtonEvent(lv_event_t* event) {
    auto* row = static_cast<lv_obj_t*>(lv_event_get_target(event));
    auto* modal = lv_obj_get_parent(row);
    auto* screen = static_cast<WeatherScreen*>(lv_obj_get_user_data(modal));
    for (size_t index = 0; index < screen->modalRows_.size(); ++index) {
        if (screen->modalRows_[index] != row) continue;
        const int city = screen->modalRowCityIndices_[index];
        if (city < 0) return;
        screen->cityIndex_ = city;
        screen->setModalVisible(false);
        screen->selectCity(city);
        return;
    }
}
