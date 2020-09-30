#include "../../include/util/message.hpp"
#include <sstream>
#include <iostream>

namespace message{


Message::Message() {

}

Message::Message(string text, string user, string group, long int time) {
    this->text = text;
    this->user = user;
    this->group = group;
    this->time = time;
}

string Message::getText() {
    return this->text;
}
string Message::getUser() {
    return this->user;
}
string Message::getGroup(){
    return this->group;
}
long int Message::getTime(){
    return this->time;
}

void Message::setText(string text) {
    this->text = text;
}
void Message::setUser(string user) {
    this->user = user;
}
void Message::setGroup(string group) {
    this->group = group;
}
void Message::setTime(long int time) {
    this->time = time;
}


} // namespace message;