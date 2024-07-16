#include <iostream>
#include <string>
#include <curl/curl.h>
#include <ctime>
#include <unordered_map>
#include "json.hpp"

using json = nlohmann::json;

std::unordered_map<std::string, std::string> cache;
std::string apiKey = "YOUR_API_KEY"; // Replace with your actual API key

std::string weatherData;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

bool fetchWeatherData(const std::string& apiKey, const std::string& city, const std::string& units) {
    CURL* curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        std::string url = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "&appid=" + apiKey + "&units=" + units;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &weatherData);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return false;
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return true;
}

void displayWeatherData(const std::string& data) {
    try {
        auto jsonData = json::parse(data);

        std::string city = jsonData["name"];
        std::string weather = jsonData["weather"][0]["description"];
        double temp = jsonData["main"]["temp"];
        double feels_like = jsonData["main"]["feels_like"];
        int humidity = jsonData["main"]["humidity"];
        double wind_speed = jsonData["wind"]["speed"];
        int pressure = jsonData["main"]["pressure"];
        std::time_t sunrise = jsonData["sys"]["sunrise"];
        std::time_t sunset = jsonData["sys"]["sunset"];

        std::cout << "\033[1;32m\n\n=====================\n";
        std::cout << " Weather in " << city << "\n";
        std::cout << "=====================\n\033[0m";
        std::cout << "\033[1;34mDescription: \033[0m" << weather << "\n";
        std::cout << "\033[1;34mTemperature: \033[0m" << temp << " °C\n";
        std::cout << "\033[1;34mFeels Like: \033[0m" << feels_like << " °C\n";
        std::cout << "\033[1;34mHumidity: \033[0m" << humidity << " %\n";
        std::cout << "\033[1;34mWind Speed: \033[0m" << wind_speed << " m/s\n";
        std::cout << "\033[1;34mPressure: \033[0m" << pressure << " hPa\n";
        std::cout << "\033[1;34mSunrise: \033[0m" << std::ctime(&sunrise);
        std::cout << "\033[1;34mSunset: \033[0m" << std::ctime(&sunset);
        std::cout << "\033[1;32m=====================\n\n\033[0m";
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
    }
}

bool isValidCityName(const std::string& city) {
    return !city.empty() && city.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ ") == std::string::npos;
}

int main() {
    std::string city;
    std::string units;

    std::cout << R"(
  __        __   _                            _          _   _             
  \ \      / /__| | ___ ___  _ __ ___   ___  | |_ ___   | |_| |_   _  ___  
   \ \ /\ / / _ \ |/ __/ _ \| '_ ` _ \ / _ \ | __/ _ \  | __| | | | |/ _ \ 
    \ V  V /  __/ | (_| (_) | | | | | |  __/ | || (_) | | |_| | |_| |  __/ 
     \_/\_/ \___|_|\___\___/|_| |_| |_|\___|  \__\___/   \__|_|\__,_|\___| 
                                                                          
    )" << std::endl;

    std::cout << "Enter city name: ";
    std::getline(std::cin, city);

    while (!isValidCityName(city)) {
        std::cerr << "Invalid city name. Please enter a valid city name: ";
        std::getline(std::cin, city);
    }

    std::cout << "Choose units (metric/imperial): ";
    std::getline(std::cin, units);

    while (units != "metric" && units != "imperial") {
        std::cerr << "Invalid choice. Please enter 'metric' or 'imperial': ";
        std::getline(std::cin, units);
    }

    if (cache.find(city) != cache.end()) {
        weatherData = cache[city];
    } else {
        if (fetchWeatherData(apiKey, city, units)) {
            cache[city] = weatherData;
        } else {
            std::cerr << "Failed to retrieve weather data." << std::endl;
            return 1;
        }
    }

    displayWeatherData(weatherData);

    return 0;
}

