#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std;

const string BOT_TOKEN = "8769381388:AAHE7NFRWJM3RDYNjQvggaw8QyaRkqP7u0";
const string ADMIN_CHAT_ID = "8888320555";

size_t WriteCallback(void *contents, size_t size, size_t nmemb, string *output) {
    size_t total = size * nmemb;
    output->append((char*)contents, total);
    return total;
}

string httpGet(const string &url) {
    CURL *curl = curl_easy_init();
    string response;
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return response;
}

string httpPost(const string &url, const string &postData) {
    CURL *curl = curl_easy_init();
    string response;
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return response;
}

bool sendMessage(const string &chatId, const string &text) {
    string url = "https://api.telegram.org/bot" + BOT_TOKEN + "/sendMessage";
    char *escaped = curl_easy_escape(nullptr, text.c_str(), 0);
    string postData = "chat_id=" + chatId + "&text=" + escaped;
    curl_free(escaped);
    string response = httpPost(url, postData);
    try {
        json j = json::parse(response);
        return j["ok"].get<bool>();
    } catch (...) {
        return false;
    }
}

void processUpdate(const json &update) {
    if (!update.contains("message")) return;
    auto msg = update["message"];
    if (!msg.contains("text")) return;
    string text = msg["text"];
    string chatId = to_string(msg["chat"]["id"].get<int>());

    if (chatId != ADMIN_CHAT_ID) {
        sendMessage(chatId, "Access denied.");
        return;
    }

    if (text == "/start") {
        string menu = "CustoJusto Bot Menu:\n"
                      "/run - Start distribution\n"
                      "/status - Check status\n"
                      "/help - This message";
        sendMessage(chatId, menu);
    } else if (text == "/run") {
        sendMessage(chatId, "Distribution started (simulated).");
    } else if (text == "/status") {
        sendMessage(chatId, "Bot is running. Interval 30 min.");
    } else {
        sendMessage(chatId, "Unknown command. Use /start");
    }
}

int main() {
    curl_global_init(CURL_GLOBAL_ALL);
    int offset = 0;
    while (true) {
        string url = "https://api.telegram.org/bot" + BOT_TOKEN + "/getUpdates?offset=" + to_string(offset) + "&timeout=30";
        string response = httpGet(url);
        try {
            json j = json::parse(response);
            if (j["ok"].get<bool>()) {
                for (auto &update : j["result"]) {
                    processUpdate(update);
                    offset = update["update_id"].get<int>() + 1;
                }
            }
        } catch (const exception &e) {
            cerr << "Error: " << e.what() << endl;
        }
        this_thread::sleep_for(chrono::seconds(1));
    }
    curl_global_cleanup();
    return 0;
}
