/**
 * string의 input만 cin또는 getline을 사용합니다. 그 외는 모두 scanf, printf와 같은 c 함수를 사용합니다.
 */
#include <arpa/inet.h>
#include <limits.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <termios.h>
#include <unistd.h>

#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
using namespace std;
using json = nlohmann::json;
int get_key() {
    int ch;
    struct termios old;
    struct termios current;
    tcgetattr(0, &old);
    current = old;
    current.c_lflag &= ~ICANON;
    current.c_lflag &= ~ECHO;
    tcsetattr(0, TCSANOW, &current);
    ch = getchar();
    tcsetattr(0, TCSANOW, &old);
    return ch;
}
bool send(json send_json, int sock){
    const char *c = send_json.dump().c_str();
    send(sock, c, strlen(c), 0);
    return true;
}
bool is_shift_key(){
    int key;
    key = KEY_LEFTSHIFT;
    FILE *kbd = fopen("/dev/input/by-path/platform-i8042-serio-0-event-kbd", "r");
    if(kbd == NULL){
        perror("접근불가");
        exit(EXIT_FAILURE);
    }
    char key_map[KEY_MAX / 8 + 1];
    memset(key_map, 0, sizeof(key_map));
    ioctl(fileno(kbd), EVIOCGKEY(sizeof(key_map)), key_map);
    int keyb = key_map[key / 8];
    int mask = 1 << (key % 8);
    fclose(kbd);
    return keyb & mask;
}
int input_port() {
    int port;
    string s_port;
input_port:
    printf("포트: ");
    getline(cin, s_port);
    try {
        port = stoi(s_port);
    } catch (exception &e) {
        printf("잘못된 포트!\n");
        goto input_port;
    }
    return port;
}
string input_bot_name() {
    string bot_name;
    printf("봇 이름: ");
    getline(cin, bot_name);
    return bot_name;
}
string input_sender_name() {
    string sender_name;
    printf("전송자(DEBUG SENDER): ");
    getline(cin, sender_name);
    sender_name = sender_name == "" ? "DEBUG SENDER" : sender_name;
    return sender_name;
}
string input_room_name() {
    string room_name;
    printf("방 이름(DEBUG ROOM): ");
    getline(cin, room_name);
    room_name = room_name == "" ? "DEBUG ROOM" : room_name;
    return room_name;
}
bool input_is_group_chat() {
    string is_group_chat;
    printf("그룹쳇 입니까?(y만 true): ");
    getline(cin, is_group_chat);
    return is_group_chat == "y";
}
string input_package_name() {
    string package_name;
    printf("전송한 패키지명(com.xfl.msgbot): ");
    getline(cin, package_name);
    return package_name == "" ? "com.xfl.msgbot" : package_name;
}
int main() {
    is_shift_key();
    int port = input_port();
    json send_json, receive_json;
    send_json["name"] = "debugRoom";
    send_json["data"]["botName"] = input_bot_name();
    send_json["data"]["authorName"] = input_sender_name();
    send_json["data"]["roomName"] = input_room_name();
    send_json["data"]["isGroupChat"] = input_is_group_chat();
    send_json["data"]["packageName"] = input_package_name();
    printf("디버그룸을 시작합니다.\n·Ctrl+C키를 눌러 나갈 수 있습니다.\n·Shift+Enter키를 눌러 여러줄을 입력 할수 있습니다.\n");
    string msg;
    printf("%s: ", send_json["data"]["authorName"].get<string>().c_str());
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("Invalid address/ Address not supported");
        return -1;
    }
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection Failed");
        return -1;
    }
    while (true) {
        string gl;
        getline(cin, gl);
        if(msg == "") msg = gl;
        else msg += "\n" + gl;
        if(!is_shift_key()){
            send_json["data"]["message"] = msg;
            send(send_json, sock);
            msg = "";
            printf("%s: ", send_json["data"]["authorName"].get<string>().c_str());
        }
    }
    return EXIT_SUCCESS;
}
