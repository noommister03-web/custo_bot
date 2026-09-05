FROM gcc:latest
RUN apt-get update && apt-get install -y libcurl4-openssl-dev nlohmann-json3-dev
WORKDIR /app
COPY bot.cpp .
RUN g++ -std=c++17 bot.cpp -lcurl -pthread -o bot
CMD ["./bot"]
