#include <iostream>
#include <string>
#include <curl/curl.h>
#include "json.hpp" 

using json = nlohmann::json;

std::string weatherData;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void fetchWeatherData(const std::string& apiKey, const std::string& city) {
    CURL* curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        std::string url = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "&appid=" + apiKey + "&units=metric";

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &weatherData);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
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

        std::cout << "\n\n=====================\n";
        std::cout << " Weather in " << city << "\n";
        std::cout << "=====================\n";
        std::cout << "Description: " << weather << "\n";
        std::cout << "Temperature: " << temp << " °C\n";
        std::cout << "Feels Like: " << feels_like << " °C\n";
        std::cout << "Humidity: " << humidity << " %\n";
        std::cout << "Wind Speed: " << wind_speed << " m/s\n";
        std::cout << "=====================\n\n";
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
    }
}

int main() {
    std::string apiKey = "YOUR_API_KEY"; 
    std::string city;

    std::cout << R"(
  __        __   _                            _          _   _             
  \ \      / /__| | ___ ___  _ __ ___   ___  | |_ ___   | |_| |_   _  ___  
   \ \ /\ / / _ \ |/ __/ _ \| '_ ` _ \ / _ \ | __/ _ \  | __| | | | |/ _ \ 
    \ V  V /  __/ | (_| (_) | | | | | |  __/ | || (_) | | |_| | |_| |  __/ 
     \_/\_/ \___|_|\___\___/|_| |_| |_|\___|  \__\___/   \__|_|\__,_|\___| 
                                                                          
    )" << std::endl;

    std::cout << "Enter city name: ";
    std::getline(std::cin, city);

    fetchWeatherData(apiKey, city);

    if (!weatherData.empty()) {
        displayWeatherData(weatherData);
    } else {
        std::cout << "Failed to retrieve weather data." << std::endl;
    }

    return 0;
}
