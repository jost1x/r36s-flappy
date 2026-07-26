#include <algorithm>

#include "weather/weather_client.h"

namespace {
constexpr std::array<WeatherLocation, 18> kLocations{{
    {"santiago", "Santiago", "Chile", -33.4489, -70.6693},
    {"arica", "Arica", "Chile", -18.4783, -70.3126},
    {"antofagasta", "Antofagasta", "Chile", -23.6509, -70.3323},
    {"copiapo", "Copiapó", "Chile", -27.3668, -70.3323},
    {"la-serena", "La Serena", "Chile", -29.9027, -71.2519},
    {"valparaiso", "Valparaíso", "Chile", -33.0472, -71.6127},
    {"rancagua", "Rancagua", "Chile", -34.1708, -70.7444},
    {"talca", "Talca", "Chile", -35.4264, -71.6662},
    {"concepcion", "Concepción", "Chile", -36.8201, -73.0444},
    {"temuco", "Temuco", "Chile", -38.7359, -73.0444},
    {"valdivia", "Valdivia", "Chile", -39.8142, -73.2459},
    {"osorno", "Osorno", "Chile", -40.5739, -73.1335},
    {"puerto-montt", "Puerto Montt", "Chile", -41.4693, -72.9424},
    {"coyhaique", "Coyhaique", "Chile", -45.5752, -72.0662},
    {"punta-arenas", "Punta Arenas", "Chile", -53.1638, -70.9171},
    {"iquique", "Iquique", "Chile", -20.2307, -70.1357},
    {"chillan", "Chillán", "Chile", -36.6066, -72.1034},
    {"castro", "Castro", "Chile", -42.4721, -73.7732},
}};
}

const std::array<WeatherLocation, 18>& weatherLocations() { return kLocations; }
const WeatherLocation* weatherLocationById(const std::string& id) {
    const auto found =
        std::find_if(kLocations.begin(), kLocations.end(), [&id](const auto& location) { return id == location.id; });
    return found == kLocations.end() ? nullptr : &*found;
}
const char* weatherDescription(int code) {
    if (code == 0) return "Despejado";
    if (code <= 2) return "Parcialmente nublado";
    if (code == 3) return "Nublado";
    if (code == 45 || code == 48) return "Niebla";
    if (code >= 51 && code <= 57) return "Llovizna";
    if ((code >= 61 && code <= 67) || (code >= 80 && code <= 82)) return "Lluvia";
    if (code >= 71 && code <= 77) return "Nieve";
    if (code >= 95) return "Tormenta";
    return "Sin datos";
}
const char* weatherSymbol(int code) {
    return code == 0 ? "*" : code <= 2 ? "o~" : code == 3 ? "~" : code >= 95 ? "!" : "+";
}
bool isStormyOrRainy(int code) { return (code >= 51 && code <= 67) || (code >= 80 && code <= 99) || code == 3; }
