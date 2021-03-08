//
// Created by 田地 on 2021/3/5.
//

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <arpa/inet.h>
#include "protocol.h"
#include "io.h"

// ===================================
//             Attribute
// ===================================

bool SBCP::Attribute::IsValidType(Type type) {
    return (type <= ST) && (type >= END);
}

SBCP::Attribute::Attribute(SBCP::Attribute::Type _type, uint16_t _len, void* data)
: type(_type),
  len(_len),
  payload((char*)data, (char*) data + _len)
  {}

SBCP::Attribute::Attribute(SBCP::Attribute::Type _type, const std::string& data)
: type(_type),
  len(data.size()),
  payload(data.data(), data.data() + data.size())
  {}

SBCP::Attribute::Attribute(void* buf) {
    ReadBytes(buf);
}

SBCP::Attribute::Attribute(const SBCP::Attribute& attr)
: type(attr.type),
  len(attr.len),
  payload(attr.payload)
  {}

SBCP::Attribute::Attribute(SBCP::Attribute&& attr)
: type(attr.type),
  len(attr.len),
  payload(std::move(attr.payload))
  {}

uint32_t SBCP::Attribute::WriteHeader(void* dst) {
    *((uint16_t *) dst) = htons(uint16_t (type));
    *((uint16_t *) dst + 1) = htons(len);
    return ATTR_HEADER_SIZE;
}

uint32_t SBCP::Attribute::WritePayload(void* dst) {
    memcpy(dst, payload.data(), len);
    return len;
}

uint32_t SBCP::Attribute::WriteBytes(void* dst) {
    uint32_t hSize = WriteHeader(dst);
    return hSize + WritePayload( (void*) ((char *)dst + hSize) );
}

uint32_t SBCP::Attribute::ReadHeader(void* buf) {
    type = Type(ntohs(*((uint16_t *) buf)));
    if (IsValidType(type)) {
        throw std::runtime_error("invalid message header" + std::to_string(type));
    }
    len = ntohs(*((uint16_t *) buf + 1));
    return ATTR_HEADER_SIZE;
}

uint32_t SBCP::Attribute::ReadPayload(void* buf) {
    payload = std::vector<char>((char *) buf, (char *) buf + len);
    return len;
}

uint32_t SBCP::Attribute::ReadBytes(void* buf) {
    uint32_t hSize = ReadHeader(buf);
    return hSize + ReadPayload((char *) buf + hSize);
}

SBCP::Attribute::Type SBCP::Attribute::GetType() const { return type; }

uint16_t SBCP::Attribute::GetLen() const { return len; }

std::vector<char> SBCP::Attribute::GetPayload() const { return payload; }

std::string SBCP::Attribute::GetPayloadString() const { return std::string(payload.begin(), payload.end()); }

void* SBCP::Attribute::PayloadPtr() const { return (void*)payload.data(); }

uint32_t SBCP::Attribute::Size() const { return MSG_HEADER_SIZE + len; }

bool SBCP::Attribute::operator==(const SBCP::Attribute& other) const {
    return (type == other.GetType()) && (len == other.GetLen()) && (payload == other.GetPayload());
}

std::ostream& SBCP::operator<<(std::ostream& os, const SBCP::Attribute& attr) {
    os<< "Header: "
      << "Type: " << attr.GetType()
      << ", Len: " << attr.GetLen() << "\n"
      << "Data: " << std::string((char *) attr.PayloadPtr(), (char *) attr.PayloadPtr() + attr.GetLen() )
      << "\n";
    return os;
}

// ===================================
//               Message
// ===================================

SBCP::Message::Message(SBCP::Message::Type _type, uint16_t _ver, SBCP::Message::AttrMap&& _attrMap)
: type(_type),
  ver(_ver),
  len(0),
  attrMap(_attrMap) {

    for (auto& it : attrMap) {
        len += it.second.Size();
    }

}

SBCP::Message::Message(void* buf, uint32_t size) {
    ReadBytes(buf, size);
}

uint32_t SBCP::Message::GetVer() const { return ver; }

uint32_t SBCP::Message::GetType() const { return type; }

uint32_t SBCP::Message::GetLen() const { return len; }

const SBCP::Message::AttrMap& SBCP::Message::GetAttrMap() const { return attrMap; }

uint32_t SBCP::Message::Size() const { return 4 + len; }

void SBCP::Message::SetAttr(Attribute attr) {
    len += attr.Size();
    attrMap.insert({attr.GetType(), std::move(attr)});
}

uint32_t SBCP::Message::WriteHeader(void* dst) {
    uint16_t verAndType = (uint16_t (ver) & 0x1FF) | ((uint16_t (type) & 0x7F) << 9);
    *((uint16_t *) dst) = htons(verAndType);
    *((uint16_t *) dst + 1) = htons(len);
    return MSG_HEADER_SIZE;
}

uint32_t SBCP::Message::WriteAttrs(void* dst) {
    uint32_t cur = 0;
    for (auto& it : attrMap) {
        cur += it.second.WriteBytes( (void*)((char *)dst + cur) );
    }
    return cur;
}

uint32_t SBCP::Message::WriteBytes(void* dst) {
    uint32_t hSize = WriteHeader(dst);
    return hSize + WriteAttrs( (void *) ((char *)dst + hSize) );
}

uint32_t SBCP::Message::ReadHeader(void* buf) {
    uint16_t verAndType = ntohs(*((uint16_t *) buf));
    len = ntohs(*((uint16_t *) buf + 1));
    ver = (verAndType & 0x1FF);
    type = Type((verAndType & 0xFE00) >> 9);
    if (type <= ST && type >= END) {
        throw std::runtime_error("invalid message header" + std::to_string(type));
    }
    return MSG_HEADER_SIZE;
}

uint32_t SBCP::Message::ReadPayload(void* buf) {
    uint32_t cur = 0;
    attrMap.clear();
    while (cur < len) {
        Attribute attr;
        cur += attr.ReadHeader((char *) buf + cur);
        if (attr.GetLen() + cur > len) {
            throw std::runtime_error("invalid message format");
        }
        cur += attr.ReadPayload((char *) buf + cur);
        attrMap.insert({attr.GetType(), std::move(attr)});
    }
    return cur;
}

ssize_t SBCP::Message::ReadBytes(void* buf, uint32_t maxSize) {
    if (maxSize < 4) {
        throw std::runtime_error("invalid maxSize, must be at least 4");
    }
    ssize_t cur = ReadHeader(buf);
    cur += ReadPayload((char *) buf + cur);
    return cur;
}

bool SBCP::Message::operator==(const Message& other) {
    return (type == other.GetType()) &&
           (ver == other.GetVer()) &&
           (len == other.GetLen()) &&
           (attrMap == other.GetAttrMap());
}

std::ostream& SBCP::operator<<(std::ostream& os, const SBCP::Message& msg) {
    os<< "-----------------------------\n";
    os<< "Message: Type: " << msg.GetType()
      << ", Version: " << msg.GetVer()
      << ", Length: " << msg.GetLen() << "\n"
      << "Payload: " << "\n\n";
    uint32_t i = 1;
    for (auto& it : msg.GetAttrMap()) {
        os<< "Attribute " << i << "\n"
          << it.second << "\n";
        i++;
    }
    os<< "-----------------------------\n";
    return os;
}

// ===================================
//      Message Helper Functions
// ===================================

SBCP::Message SBCP::NewJoinMessage(const std::string& username) {
    Attribute attrUsername(Attribute::Username, username);
    Message msg(Message::JOIN, PROTOCOL_VERSION, {{attrUsername.GetType(), attrUsername}});
    return msg;
}

SBCP::Message SBCP::NewFWDMessage(const std::string& username, const std::string& message) {
    Attribute attrUsername(Attribute::Username, username);
    Attribute attrMessage(Attribute::Message, message);
    Message msg(Message::FWD, PROTOCOL_VERSION, {
        {attrUsername.GetType(), attrUsername},
        {attrMessage.GetType(), attrMessage},
    });
    return msg;
}

SBCP::Message SBCP::NewSendMessage(const std::string& payload) {
    Attribute attrMsg(Attribute::Message,  payload);
    Message msg(Message::SEND, PROTOCOL_VERSION, {{attrMsg.GetType(), attrMsg}});
    return msg;
}

ssize_t SBCP::ReadMessage(Message& msg, int fd) {
    int readl = 0;
    char header[4];

    if((readl = SBCP::readlen(fd, header, 4)) <= 0) {
        return readl;
    }

    msg.ReadHeader(header);

    char* payload = new char [msg.GetLen()];

    if((readl = SBCP::readlen(fd, payload, msg.GetLen())) <= 0) {
        return readl;
    }

    msg.ReadPayload(payload);

    return msg.Size();
}
