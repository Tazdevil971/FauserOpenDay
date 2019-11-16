#include <iostream>
#include <map>

#include "Serial.hpp"
#include "Curl.hpp"

#define BOT_TOKEN "bot1027201618:AAEzajVZgk_0ErqC-TmCuUbZ1xbwO9kfE_Q"
#define METHOD_URL "https://api.telegram.org/"BOT_TOKEN"/"
#define FILE_URL "https://api.telegram.org/file/"BOT_TOKEN"/"

class App {
public:
	App(std::string comName) :
		m_curl(),
		m_serial(comName) {}

	void run() {
		testBot();

		m_serial.begin(115200);
		while(m_serial.available() != 0) 
			m_serial.read();

		nlohmann::json parameters_getUpdates;
		parameters_getUpdates["allowed_updates"] = {
			"message",
			"callback_query"
		};

		int offset = 0;

		std::cout << std::endl << "ready!" << std::endl << std::endl;
		while(true) {
			parameters_getUpdates["offset"] = offset;
			
			nlohmann::json result_getUpdates;
			if(!m_curl.request(METHOD_URL"getUpdates", result_getUpdates, parameters_getUpdates))
				throw std::exception("getUpdates failed!");
			
			result_getUpdates = result_getUpdates["result"];
			for(auto it = result_getUpdates.begin(); it != result_getUpdates.end(); it++) {
				offset = it->operator[]("update_id").get<int>() + 1;

				auto content = it->find("message");
				if(content != it->end())
					handleMessage(*content);
				
				content = it->find("callback_query");
				if(content != it->end())
					handleCallbackQuery(*content);
			}

			m_serial.print('r');
			while(m_serial.available() != 4);
			
			short newTemperature = m_serial.read() | m_serial.read() << 8;
			short newHumidity = m_serial.read() | m_serial.read() << 8;


			if(newTemperature != m_temperature || newHumidity != m_humidity) {
				m_temperature = newTemperature;
				m_humidity = newHumidity;

				for(auto it = m_chats.cbegin(); it != m_chats.cend(); it++)
					m_chats[it->first] = editMessage(it->first, it->second, getSensorString(), true);
			}

			Sleep(5);
		}
	}

private:
	void testBot() {
		nlohmann::json result;
		if(!m_curl.request(METHOD_URL"getMe", result))
			throw std::exception("getMe failed!");

		
		result = result["result"];

		std::cout
			<< "id: " << result["id"] << std::endl
			<< "username: @" << result["username"].get<std::string>() << std::endl
			<< "name: " << result["first_name"].get<std::string>() << std::endl;
	}
	
	void handleMessage(nlohmann::json& message) {
		std::string text = message["text"];
		int chatId = message["chat"]["id"];
		std::cout << "received message from: " << chatId << " text: " << text << std::endl;
		
		bool userExists = m_chats.count(chatId) != 0;
		if(text == "/start" && !userExists) {
			std::cout << "sending welcome..." << std::endl;
			sendMessage(chatId, "Welcome!!", false);

			std::cout << "registed chat " << chatId << " message: " << (m_chats[chatId] = sendMessage(chatId, getSensorString(), true)) << std::endl;
		
		} else if(text == "/stop" && userExists) {
			std::cout << "sending goodbye" << std::endl;
			sendMessage(chatId, "Goodbye!!", false);

			m_chats.erase(chatId);
			std::cout << "removed chat " << chatId << std::endl;
		
		} else if(userExists) {
			std::cout << "updated message: " << (m_chats[chatId] = sendMessage(chatId, getSensorString(), true)) << std::endl;
		}
	}

	void handleCallbackQuery(nlohmann::json& callbackQuery) {
		nlohmann::json parameters;
		nlohmann::json result;
		
		if(m_chats.count(callbackQuery["message"]["chat"]["id"]) != 0) {
			
			bool command = callbackQuery["data"] == "on";
			if(command) {
				if(m_temperature > 22) {
					std::cout << "powering relay..." << std::endl;
					m_serial.print('h');

					parameters["text"] = "Done!";
				} else {
					parameters["text"] = "Temperature too low!";
				}
			} else {
				if(m_temperature < 26) {
					std::cout << "disabling relay..." << std::endl;
					m_serial.print('l');

					parameters["text"] = "Done!";
				} else {
					parameters["text"] = "Temperature too high!";
				}
			}
		}
		
		parameters["callback_query_id"] = callbackQuery["id"];
		if(!m_curl.request(METHOD_URL"answerCallbackQuery", result, parameters))
			throw std::exception("answerCallbackQuery failed");
	}

	int sendMessage(int chatId, std::string text, bool sendButtons) {
		nlohmann::json parameters;
		nlohmann::json result;

		parameters["chat_id"] = chatId;
		parameters["text"] = text;
		if(sendButtons)
			parameters["reply_markup"] = m_replyMarkup;

		if(!m_curl.request(METHOD_URL"sendMessage", result, parameters))
			throw std::exception("sendMessage failed");

		return result["result"]["message_id"];
	}

	int editMessage(int chatId, int messageId, std::string text, bool sendButtons) {
		nlohmann::json parameters;
		nlohmann::json result;

		parameters["chat_id"] = chatId;
		parameters["message_id"] = messageId;
		parameters["text"] = text;
		if(sendButtons)
			parameters["reply_markup"] = m_replyMarkup;

		if(!m_curl.request(METHOD_URL"editMessageText", result, parameters))
			throw std::exception("editMessageText failed");

		return result["result"]["message_id"];
	}

	std::string getSensorString() {
		return "Temperature: " + std::to_string(m_temperature) + " humidity: " + std::to_string(m_humidity);
	}

	std::map<int, int> m_chats;
	const nlohmann::json m_replyMarkup = "{\"inline_keyboard\": [[{\"text\":\"ON\", \"callback_data\":\"on\"}, {\"text\":\"OFF\", \"callback_data\":\"off\"}]]}"_json;

	int m_temperature;
	int m_humidity;

	Serial m_serial;
	Curl m_curl;
};

int main(int argc, char** argv) {

	if (argc != 2) {
		std::cout << "Wrong number of arguments" << std::endl;
		return EXIT_FAILURE;
	}

	try { 
		App app(argv[1]);
		app.run(); 
	} catch(std::exception& e) {
		std::cout << e.what() << std::endl;
		
		std::cout << "Press any button to continue..." << std::endl;
		std::getchar();
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}